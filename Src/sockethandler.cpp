//
// Created by 20852 on 25-7-27.
//

#include "sockethandler.h"
#include <QDebug>

SocketHandler::SocketHandler(QTcpSocket *socket, QObject *parent)
    : QObject(parent), mainTcpSocket(socket), m_blockSize(0)
{
    connect(mainTcpSocket, &QTcpSocket::readyRead, this, &SocketHandler::onSocketReadyRead);
    connect(mainTcpSocket, &QTcpSocket::disconnected, this, &SocketHandler::onSocketDisconnected);
    connect(mainTcpSocket, &QTcpSocket::errorOccurred, this, &SocketHandler::onSocketErrorOccurred);
}

SocketHandler::~SocketHandler() {}

void SocketHandler::sendJsonToServer(const QJsonObject &jsonObj) {
    QByteArray jsonData = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
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

void SocketHandler::onSocketReadyRead() {
    QDataStream in(mainTcpSocket);
    in.setVersion(QDataStream::Qt_6_0);

    for (;;) {
        if (m_blockSize == 0) {
            if (mainTcpSocket->bytesAvailable() < (qint64)sizeof(quint32)) {
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
}

void SocketHandler::onSocketDisconnected() {
    qWarning() << "与服务器断开连接";
}

void SocketHandler::onSocketErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    qCritical() << "Socket错误：" << mainTcpSocket->errorString();
}