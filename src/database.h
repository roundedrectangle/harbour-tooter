#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSettings>

class Database : public QObject {
    Q_OBJECT

    Q_PROPERTY(notificationLastId READ notificationLastId WRITE setNotificationLastId NOTIFY notificationLastIdChanged)
    Q_PROPERTY(accounts READ accounts WRITE setAccounts NOTIFY accountsChanged)

public:
    explicit Database(QObject *parent = nullptr);

    int notificationLastId();
    void setNotificationLastId(int value);

    QVariantList accounts();
    void setAccounts(const QVariantList &value);
    void addAccount(const QVariantMap &account);

signals:
    void notificationLastIdChanged();
    void accountsChanged();

private:
    void migrateStorage();

private:
    QSettings *settings;
};

#endif // DATABASE_H
