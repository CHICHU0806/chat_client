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
    void sendRawData(const QByteArray& data);  //发送原始数据的方法
    QTcpSocket* getSocket() { return mainTcpSocket; }

signals:
    void registerResponse(const QJsonObject& response);
    void loginResponse(const QJsonObject& response);
    void chatMessageReceived(const QJsonObject& message);
    void userListUpdated(const QJsonObject& response);
    void userInfoUpdateResponse(const QJsonObject& response);
    void searchFriendResponse(const QJsonObject& response);
    void addFriendResponse(const QJsonObject& response);
    void friendRequestReceived(const QJsonObject& request);
    void friendRequestListReceived(const QJsonObject& response);
    void offlineMessagesReceived(const QJsonObject& response);
    void acceptFriendRequestResponse(const QJsonObject& response);
    void rejectFriendRequestResponse(const QJsonObject& response);
    void friendRequestNotificationReceived(const QJsonObject& notification);
    void friendListReceived(const QJsonObject& response);
    void aiAnswerReceived(const QJsonObject& response);
    void fileTransferResponse(const QJsonObject& response);
    void fileChunkReceived(const QByteArray& data);
    void unknownMessageReceived(const QJsonObject& obj); // 未知消息类型信号

private:
    explicit NetworkManager(QObject* parent = nullptr);
    void processResponse(const QJsonObject& response);

    QTcpSocket* mainTcpSocket;
    quint32 m_blockSize;
    static NetworkManager* _instance;
};

#endif //NETWORKMANAGER_H
