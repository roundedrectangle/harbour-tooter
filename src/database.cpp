#include "database.h"

#include <QtSql>
#include <QJsonDocument>

namespace {
    const QString NOTIFICAITON_LAST_ID("notificationLastId");
    const QString ACCOUNTS("accounts");
}

Database::Database(QObject *parent) : QObject(parent) {
    migrateStorage();

    const QUrl appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    settings = new QSettings(appData.resolved(QUrl("settings.conf")).toString(), QSettings::NativeFormat);
}

void Database::migrateStorage() {
    const QDir appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir oldStorageDirectory = appData;
    oldStorageDirectory.cd("QML/OfflineStorage/Databases/");
    if (!oldStorageDirectory.exists()) return;

    const QFileInfoList entries = oldStorageDirectory.entryInfoList(QStringList{"*.sqlite"}, QDir::Files);
    if (entries.isEmpty()) return;

    const QString oldDatabasePath = entries.first().absoluteFilePath();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "migration");
    db.setDatabaseName(oldDatabasePath);

    if (db.open()) {
        QSqlQuery query = db.exec("SELECT * FROM settings;");
        while (query.next()) {
            const QString key = query.value(0).toString();
            const QVariant value = QJsonDocument::fromJson(query.value(0).toByteArray()).toVariant();

            if (key == "notificationLastID")
                setNotificationLastId(value.toInt());
        }
    }

    QSqlDatabase::removeDatabase("migration");

    QDir oldStorageRoot = appData;
    oldStorageRoot.cd("QML");
    oldStorageRoot.removeRecursively();
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

QVariantList Database::accounts() {
    return settings->value(ACCOUNTS).toList();
}
void Database::setAccounts(const QVariantList &value) {
    if (accounts() != value) {
        settings->setValue(ACCOUNTS, value);
        emit accountsChanged();
    }
}
void Database::addAccount(const QVariantMap &account) {
// TODO: beginReadArray, beginWriteArray, ...
}
