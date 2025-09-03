//
// Created by 20852 on 25-7-22.
//

#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket> // 确保包含 QTcpSocket 头文件
#include <QJsonDocument> // 用于构建 JSON 请求
#include <QJsonObject>   // 用于构建 JSON 请求
#include <QMessageBox>   // 用于提示消息
#include <QDebug>        // 用于调试输出


class RegisterWindow : public QDialog{
    Q_OBJECT // 启用 Qt 的元对象系统

public:
    explicit RegisterWindow(QTcpSocket *socket, QWidget *parent = nullptr); // 构造函数
    ~RegisterWindow(); // 析构函数

    signals:
    void registerWindowClosed(); // 定义一个信号，表示注册窗口即将关闭

private slots:
    void onConfirmButtonClicked(); // 响应注册按钮点击的槽函数
    void handleRegisterResponse(const QJsonObject& response); // 处理注册响应的槽函数

protected:
    void closeEvent(QCloseEvent *event) override;// 重写 closeEvent 方法，在窗口关闭事件发生时发出信号

    // 定义窗口中将使用的 GUI 控件的成员变量
    QLabel *usernameLabel;
    QLineEdit *usernameLineEdit;
    QLabel *accountLabel;
    QLineEdit *accountLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QPushButton *confirmButton;

private:
    QTcpSocket *mainTcpSocket;
    quint32     m_blockSize; // 用于存储当前数据包的预期总长度
};

#endif //REGISTERWINDOW_H
