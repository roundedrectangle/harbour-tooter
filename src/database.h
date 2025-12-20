#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSettings>

class Database : public QObject {
    Q_OBJECT

    Q_PROPERTY(int notificationLastId READ notificationLastId WRITE setNotificationLastId NOTIFY notificationLastIdChanged)
    Q_PROPERTY(bool loadImages READ loadImages WRITE setLoadImages NOTIFY loadImagesChanged)
    Q_PROPERTY(bool multipleAccountsHintCompleted READ multipleAccountsHintCompleted WRITE setMultipleAccountsHintCompleted NOTIFY multipleAccountsHintCompletedChanged)

    Q_PROPERTY(QVariantList accounts READ accounts NOTIFY accountsChanged)
    Q_PROPERTY(int accountsCount READ accountsCount NOTIFY accountsCountChanged)
    Q_PROPERTY(int activeAccountIndex READ activeAccountIndex WRITE setActiveAccountIndex NOTIFY activeAccountIndexChanged)
    Q_PROPERTY(QVariantMap activeAccount READ activeAccount NOTIFY activeAccountChanged)

public:
    explicit Database(QObject *parent = nullptr);

    Q_INVOKABLE QVariantMap dumpForWorker();

    int notificationLastId();
    void setNotificationLastId(int value);

    bool loadImages();
    void setLoadImages(bool value);

    bool multipleAccountsHintCompleted();
    void setMultipleAccountsHintCompleted(bool value);

    QVariantList accounts();
    Q_INVOKABLE void addAccount(const QVariantMap &account);
    Q_INVOKABLE void removeAccount(int index);
    int accountsCount();
    int activeAccountIndex();
    void setActiveAccountIndex(int value);
    QVariantMap activeAccount();
    Q_INVOKABLE void updateActiveAccount(const QVariantMap &updated);
    Q_INVOKABLE void activateLastAccount();

signals:
    void notificationLastIdChanged();
    void loadImagesChanged();
    void multipleAccountsHintCompletedChanged();

    void accountsChanged();
    void accountsCountChanged();
    void activeAccountIndexChanged();
    void activeAccountChanged();

private:
    void migrateStorage();

private:
    QSettings *settings;
};

#endif // DATABASE_H
