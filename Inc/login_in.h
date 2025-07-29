//
// Created by 20852 on 25-7-21.
//

#ifndef LOGIN_IN_H
#define LOGIN_IN_H

#include "sockethandler.h"
#include <QWidget>     // LoginWindow 继承自 QWidget
#include <QLabel>      // 成员变量类型：QLabel*
#include <QLineEdit>   // 成员变量类型：QLineEdit*
#include <QPushButton> // 成员变量类型：QPushButton*
#include <QTcpSocket>  // TCP 客户端套接字
#include <QHostAddress> // 用于解析 IP 地址
// **新增：网络数据解析所需的头文件**
#include <QDataStream>   // 用于数据的序列化和反序列化
#include <QJsonDocument> // 用于处理 JSON 文档
#include <QJsonObject>   // 用于处理 JSON 对象
#include <QMessageBox>   // 用于显示消息框（如注册成功/失败提示）
#include <QDebug>        // 用于调试输出

#include "registerwindow.h"

class LoginWindow : public QWidget,public SocketHandler {
    Q_OBJECT // 启用 Qt 的元对象系统（信号与槽等）

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() ;

    signals:
    void registerWindowClosed();

protected:
    void processResponse(const QJsonObject &response) override;

private slots:
    void onLoginButtonClicked(); // 响应登录按钮点击的槽函数
    void onRegisterButtonClicked(); // 响应注册按钮点击的槽函数
    void handleRegisterWindowClosed(); //关闭注册后返回登录界面的槽函数
    void onConnected();             // 连接成功
    void onDisconnected();          // 断开连接
    void onSocketReadyRead();             // **新增：当有新数据可读时触发**
    void onSocketErrorOccurred(QTcpSocket::SocketError socketError); // 发生错误

private:
    // 定义窗口中将使用的 GUI 控件的成员变量
    QLabel *accountLabel;
    QLineEdit *accountLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;

    // **新增：QTcpSocket 实例**
    QTcpSocket *mainTcpSocket;    // 用于与服务器通信的 TCP 套接字
    // **新增：客户端数据包长度，用于分包接收**
    quint32 m_blockSize;

    void connectToServer(); // **新增：连接服务器的辅助函数**
};

#endif // LOGIN_IN_H