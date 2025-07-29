//
// Created by 20852 on 25-7-27.
//

#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>

class SocketHandler : public QObject {
    Q_OBJECT

protected:
    QTcpSocket *mainTcpSocket;
    quint32 m_blockSize;

    explicit SocketHandler(QTcpSocket *socket, QObject *parent = nullptr);
    virtual ~SocketHandler();

    // 共用的数据发送方法
    void sendJsonToServer(const QJsonObject &jsonObj);

    // 纯虚函数，子类必须实现
    virtual void processResponse(const QJsonObject& response) = 0;

protected slots:
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onSocketErrorOccurred(QAbstractSocket::SocketError socketError);
};

#endif // SOCKETHANDLER_H