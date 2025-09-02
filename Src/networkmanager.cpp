//
// Created by 20852 on 25-7-31.
//

#include "networkmanager.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonObject>
#include <QHostAddress>

NetworkManager* NetworkManager::_instance = nullptr;

//单例模式设计，保证一机一ip/端口
NetworkManager* NetworkManager::instance() {
    if (!_instance) {
        _instance = new NetworkManager();
    }
    return _instance;
}

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent),
      mainTcpSocket(new QTcpSocket(this)),
      m_blockSize(0)
{
    // 设置Socket连接
    connect(mainTcpSocket, &QTcpSocket::readyRead, this, [this]() {
        QDataStream in(mainTcpSocket);
        in.setVersion(QDataStream::Qt_6_0);
        for (;;) {
            if (m_blockSize == 0) {
                if (mainTcpSocket->bytesAvailable() < sizeof(quint32)) {
                    return;
                }
                in >> m_blockSize;
            }

            if (mainTcpSocket->bytesAvailable() < m_blockSize) {
                return;
            }

            QByteArray data;
            in >> data;

            // 检查是否为文件数据包
            if (data.startsWith("FILE_CHUNK:") || data.startsWith("FILE_DATA:")) {
                emit fileChunkReceived(data);
            } else {
                // 处理JSON消息
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (!doc.isNull() && doc.isObject()) {
                    processResponse(doc.object());
                }
            }

            m_blockSize = 0;
        }
    });

    // 连接到服务器
    mainTcpSocket->connectToHost(QHostAddress("127.0.0.1"), 12345);
}

void NetworkManager::sendMessage(const QJsonObject& message) {
    QByteArray jsonData = QJsonDocument(message).toJson(QJsonDocument::Compact);

    QByteArray dataBlock;
    QDataStream out(&dataBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << (quint32)0;
    out << jsonData;

    out.device()->seek(0);
    out << (quint32)(dataBlock.size() - sizeof(quint32));

    mainTcpSocket->write(dataBlock);
    mainTcpSocket->flush();
}

void NetworkManager::sendRawData(const QByteArray& data) {
    QByteArray dataBlock;
    QDataStream out(&dataBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << (quint32)0;
    out << data;

    out.device()->seek(0);
    out << (quint32)(dataBlock.size() - sizeof(quint32));

    mainTcpSocket->write(dataBlock);
    mainTcpSocket->flush();
}

void NetworkManager::processResponse(const QJsonObject& response) {
    QString type = response["type"].toString();

    if (type == "register") {
        emit registerResponse(response);
    }
    else if (type == "login") {
        emit loginResponse(response);
    }
    else if (type == "chatMessage") {
        emit chatMessageReceived(response);
    }
    else if (type == "fileTransfer") {
        emit fileTransferResponse(response);  // 只转发，不处理
    }
    else if (type == "updateUserInfo") {
        emit userInfoUpdateResponse(response);
    }
    else if (type == "searchFriend") {
        emit searchFriendResponse(response);
    }
    else if (type == "addFriend") {
        emit addFriendResponse(response);
    }
    else if (type == "friendRequest") {
        emit friendRequestReceived(response);
    }
    else if (type == "getFriendRequests") {
        emit friendRequestListReceived(response);
    }
    else if (type == "acceptFriendRequest") {
        emit acceptFriendRequestResponse(response);
    }
    else if (type == "rejectFriendRequest") {
        emit rejectFriendRequestResponse(response);
    }
    else if (type == "friendRequestResponse") {
        emit friendRequestNotificationReceived(response);
    }
    else if (type == "getFriendList") {
        emit friendListReceived(response);
    }
    else if (type == "aiResponse") {
        emit aiAnswerReceived(response);
    }
    else if (type == "offline_messages") {
        emit offlineMessagesReceived(response);
    }
    else {
        qDebug() << "未知消息类型：" << type;
        emit unknownMessageReceived(response); // 新增：发出未知消息信号
    }
}