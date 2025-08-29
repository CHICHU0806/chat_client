//
// Created by 20852 on 25-7-21.
//

#ifndef LOGIN_IN_H
#define LOGIN_IN_H

#include "mainwindow.h"
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

class LoginWindow : public QWidget {
    Q_OBJECT // 启用 Qt 的元对象系统（信号与槽等）

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() ;

    signals:
    void registerWindowClosed();

private slots:
    void onLoginButtonClicked(); // 响应登录按钮点击的槽函数
    void onRegisterButtonClicked(); // 响应注册按钮点击的槽函数
    void handleRegisterWindowClosed(); //关闭注册后返回登录界面的槽函数
    void handleLoginResponse(const QJsonObject& response); // 处理登录响应的槽函数

private:
    MainWindow *mainWindow = nullptr;
    QLabel *accountLabel;
    QLineEdit *accountLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
};

#endif // LOGIN_IN_H