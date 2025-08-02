//
// Created by 20852 on 25-7-31.
//

#include "networkmanager.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonObject>

NetworkManager* NetworkManager::_instance = nullptr;

//单例模式设计
NetworkManager* NetworkManager::instance() {
    if (!_instance) {
        _instance = new NetworkManager();
    }
    return _instance;
}

//构造函数实现
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

            QByteArray jsonData;
            in >> jsonData;

            QJsonDocument doc = QJsonDocument::fromJson(jsonData);
            if (!doc.isNull() && doc.isObject()) {
                processResponse(doc.object());
            }

            m_blockSize = 0;
        }
    });

    // 连接到服务器
    mainTcpSocket->connectToHost(QHostAddress::LocalHost, 12345);
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
    else {
        qDebug() << "未知消息类型：" << type;
    }
}