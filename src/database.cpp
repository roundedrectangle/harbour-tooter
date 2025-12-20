#include "database.h"

#include <QtSql>
#include <QJsonDocument>
#include <QCoreApplication>

namespace {
    const QString NOTIFICAITON_LAST_ID("notificationLastId");
    const QString LOAD_IMAGES("loadImages");
    const QString MULTIPLE_ACCOUNTS_HINT_COMPLETED("multipleAccountsHintCompleted");

    const QString ACCOUNTS("accounts");
    const QString ACTIVE_ACCOUNT("activeAccount");
}

Database::Database(QObject *parent) : QObject(parent) {
    connect(this, &Database::accountsCountChanged, &Database::accountsChanged);
    connect(this, &Database::activeAccountIndexChanged, &Database::activeAccountChanged);

    const QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + QCoreApplication::organizationName() + "/" + QCoreApplication::applicationName() + "/settings.conf";
    settings = new QSettings(settingsPath, QSettings::NativeFormat);

    migrateStorage();
}

void Database::migrateStorage() {
    const QDir appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir oldStorageDirectory = appData;
    oldStorageDirectory.cd("QML/OfflineStorage/Databases");
    if (!oldStorageDirectory.exists()) return;

    const QFileInfoList entries = oldStorageDirectory.entryInfoList(QStringList{"*.sqlite"}, QDir::Files);
    if (entries.isEmpty()) return;

    const QString oldDatabasePath = entries.first().absoluteFilePath();
    qDebug() << "Migrating old configuration" << oldDatabasePath;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration");
    db.setDatabaseName(oldDatabasePath);

    bool ok = db.open();
    if (ok) {
        QSqlQuery query = db.exec("SELECT * FROM settings;");

        QVariantMap veryOldAccount;
        while (query.next()) {
            const QString key = query.value(0).toString();
            QVariant value;

            bool intOk = false;
            value = query.value(1).toString().toInt(&intOk);
            if (!intOk)
                value = QJsonDocument::fromJson(query.value(1).toByteArray()).toVariant();

            qDebug() << "Migrating" << key << value;

            if (key == "notificationLastID")
                setNotificationLastId(value.toInt());
            else if (key == MULTIPLE_ACCOUNTS_HINT_COMPLETED)
                setMultipleAccountsHintCompleted(value.toBool());
            else if (key == ACCOUNTS) {
                for (const QVariant &accountVariant : value.toList())
                    addAccount(accountVariant.toMap());
            } else if (key == "api_user_token" || key == "instance" || key == "login" || key == "type")
                // Account before the multiple accounts system was introduced
                veryOldAccount.insert(key, value);
            else
                qWarning() << "Not migrating unknown entry" << key;
        }

        if (!veryOldAccount.isEmpty())
            addAccount(veryOldAccount);
    }

    db.close();
    QSqlDatabase::removeDatabase("migration");

    if (ok) {
        QDir oldStorageRoot = appData;
        oldStorageRoot.cd("QML");
        qDebug() << "REMOVE" << oldStorageRoot;
        oldStorageRoot.removeRecursively();
    }
}

QVariantMap Database::dumpForWorker() {
    return {
        {NOTIFICAITON_LAST_ID, notificationLastId()},
        {LOAD_IMAGES, loadImages()},
        {ACTIVE_ACCOUNT, activeAccount()},
    };
}



int Database::notificationLastId() {
    return settings->value(NOTIFICAITON_LAST_ID).toInt();
}
void Database::setNotificationLastId(int value) {
    if (notificationLastId() != value) {
        settings->setValue(NOTIFICAITON_LAST_ID, value);
        emit notificationLastIdChanged();
    }
}

bool Database::loadImages() {
    return settings->value(LOAD_IMAGES, true).toBool();
}
void Database::setLoadImages(bool value) {
    if (loadImages() != value) {
        settings->setValue(LOAD_IMAGES, value);
        emit loadImagesChanged();
    }
}

bool Database::multipleAccountsHintCompleted() {
    return settings->value(MULTIPLE_ACCOUNTS_HINT_COMPLETED).toBool();
}
void Database::setMultipleAccountsHintCompleted(bool value) {
    if (multipleAccountsHintCompleted() != value) {
        settings->setValue(MULTIPLE_ACCOUNTS_HINT_COMPLETED, value);
        emit multipleAccountsHintCompletedChanged();
    }
}



QVariantList Database::accounts() {
    QVariantList accounts;
    const int size = settings->beginReadArray(ACCOUNTS);
    for (int i=1; i <= size; i++) {
        QVariantMap account;
        settings->setArrayIndex(i);
        for (const QString &key : settings->allKeys())
            account.insert(key, settings->value(key));
        accounts.append(account);
    }
    settings->endArray();
    qDebug() << "read accounts" << accounts;
    return accounts;
}

void Database::addAccount(const QVariantMap &account) {
    const int size = accountsCount();
    settings->beginWriteArray(ACCOUNTS, size + 1);
    settings->setArrayIndex(size);

    for (const QString &key : account.keys())
        settings->setValue(key, account.value(key));

    settings->endArray();
    emit accountsCountChanged();
}

void Database::removeAccount(int index) {
    const int size = accountsCount();
    settings->beginWriteArray(ACCOUNTS, size - 1);
    settings->setArrayIndex(index);
    settings->remove(""); // See QSettings::clear() documentation
    settings->endArray();
    emit accountsCountChanged();
    emit activeAccountIndexChanged();
}

int Database::accountsCount() {
    const int size = settings->beginReadArray(ACCOUNTS);
    settings->endArray();
    return size;
}

int Database::activeAccountIndex() {
    return settings->value(ACTIVE_ACCOUNT).toInt();
}

void Database::setActiveAccountIndex(int value) {
    if (activeAccountIndex() != value) {
        settings->setValue(ACTIVE_ACCOUNT, value);
        emit activeAccountIndexChanged();
    }
}

QVariantMap Database::activeAccount() {
    const int size = settings->beginReadArray(ACCOUNTS);
    if (size == 0) {
        settings->endArray();
        return QVariantMap();
    }

    int index = activeAccountIndex();
    if (index >= size) {
        index = size - 1;
        setActiveAccountIndex(index);
    }

    QVariantMap account;
    settings->setArrayIndex(index);
    for (const QString &key : settings->allKeys())
        account.insert(key, settings->value(key));
    settings->endArray();
    qDebug() << "Active account!!! (READ)" << size << index << account;
    return account;
}

void Database::updateActiveAccount(const QVariantMap &updated) {
    const int size = accountsCount();

    if (size == 0)
        return;

    int index = activeAccountIndex();
    if (index >= size) {
        setActiveAccountIndex(size - 1);
        return;
    }

    settings->beginWriteArray(ACCOUNTS, size);
    settings->setArrayIndex(index);

    for (const QString &key : updated.keys())
        settings->setValue(key, updated.value(key));

    settings->endArray();
    emit activeAccountChanged();
    emit accountsChanged();
}

void Database::activateLastAccount() {
    const int size = settings->beginReadArray(ACCOUNTS);
    settings->endArray();
    setActiveAccountIndex(size - 1);
}
