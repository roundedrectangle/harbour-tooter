#ifndef REQUESTSMANAGER_H
#define REQUESTSMANAGER_H

#include <QObject>
#include <QtNetwork>

class RequestsManager : public QObject {
    Q_OBJECT
public:
    explicit RequestsManager(QObject *parent = nullptr);
    ~RequestsManager();

    Q_INVOKABLE void setToken(const QString &newToken);
    Q_INVOKABLE void setInstance(const QUrl &instance);

    Q_INVOKABLE void get(const QString &endpoint);
signals:

private:
    QNetworkAccessManager *networkAccessManager;
    QString token;
    QUrl APIBaseUrl;

    void prepareRequest(QNetworkRequest *request, const QUrl &endpoint);
private slots:
    void handleReplyFinished(QNetworkReply *reply);
};

#endif // REQUESTSMANAGER_H
