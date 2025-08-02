//
// Created by 20852 on 25-7-31.
//

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    static NetworkManager* instance();
    void sendMessage(const QJsonObject& message);
    QTcpSocket* getSocket() { return mainTcpSocket; }

    signals:
    void registerResponse(const QJsonObject& response);
    void loginResponse(const QJsonObject& response);
    void chatMessageReceived(const QJsonObject& message);

private:
    explicit NetworkManager(QObject* parent = nullptr);
    void processResponse(const QJsonObject& response);

    QTcpSocket* mainTcpSocket;
    quint32 m_blockSize;
    static NetworkManager* _instance;
};

#endif //NETWORKMANAGER_H
