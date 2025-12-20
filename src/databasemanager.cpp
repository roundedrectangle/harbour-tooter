#include "databasemanager.h"

namespace {
    const QUrl oldStorageRootPath("QML");
    const QUrl oldStorageRelativePath("QML/OfflineStorage/Databases");
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent), database() {

}

void DatabaseManager::migrateStorage() {
    QUrl oldStorageUrl = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    oldStorageUrl = oldStorageUrl.resolved(oldStorageRelativePath);
    QDir oldStorageDirectory(oldStorageUrl.toString());

    if (!oldStorageDirectory.exists()) return;

}
