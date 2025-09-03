//
// Created by 20852 on 25-7-21.
//

#ifndef LOGIN_IN_H
#define LOGIN_IN_H

#include "mainwindow.h"
#include "registerwindow.h"
#include <QWidget>     // LoginWindow 继承自 QWidget
#include <QLabel>      // 成员变量类型：QLabel*
#include <QLineEdit>   // 成员变量类型：QLineEdit*
#include <QPushButton> // 成员变量类型：QPushButton*
#include <QTcpSocket>  // TCP 客户端套接字
#include <QHostAddress> // 用于解析 IP 地址
#include <QDataStream>   // 用于数据的序列化和反序列化
#include <QJsonDocument> // 用于处理 JSON 文档
#include <QJsonObject>   // 用于处理 JSON 对象
#include <QMessageBox>   // 用于显示消息框
#include <QDebug>        // 用于调试输出
#include <QTimer>
#include <QPainter>
#include <QVector2D>
#include <QRandomGenerator>

class LoginWindow : public QWidget {
    Q_OBJECT // 启用 Qt 的元对象系统（信号与槽等）

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() ;

    signals:
    void registerWindowClosed();

protected:
    //重写原生标题栏
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

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

    // 标题栏拖动相关成员
    QWidget *titleBar = nullptr;
    QPoint dragPosition;
    bool dragging = false;

    struct Meteor {
        QVector2D pos;
        QVector2D vel;
        float length{};
    };
    QTimer* meteorTimer = nullptr;
    QVector<Meteor> meteors;
    void resetMeteor(Meteor &meteor);
};

#endif // LOGIN_IN_H