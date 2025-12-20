#include "requestsmanager.h"

namespace {
    const QUrl relativeAPIUrl("api/v1");
}

RequestsManager::RequestsManager(QObject *parent) :
    QObject(parent),
    networkAccessManager(new QNetworkAccessManager(this))
{
    connect(networkAccessManager, &QNetworkAccessManager::finished, this, &RequestsManager::handleReplyFinished);
}

RequestsManager::~RequestsManager() {
    delete networkAccessManager;
}

void RequestsManager::setInstance(const QUrl &instance) {
    this->APIBaseUrl = instance.resolved(relativeAPIUrl);
}

void RequestsManager::setToken(const QString &newToken) {
    this->token = newToken;
}

void RequestsManager::prepareRequest(QNetworkRequest *request, const QUrl &endpoint) {
    request->setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    request->setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    request->setRawHeader("Connection", "close");

    request->setUrl(this->APIBaseUrl.resolved(endpoint));
}

void RequestsManager::handleReplyFinished(QNetworkReply *reply) {

}
