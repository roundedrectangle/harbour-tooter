#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QtSql>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

signals:

private:
    QSqlDatabase *database;
    void migrateStorage();
};

#endif // DATABASEMANAGER_H
