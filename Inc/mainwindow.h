//
// Created by 20852 on 25-7-22.
//
// mainwindow.h (客户端项目)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>      // 欢迎标签
#include <QTextEdit>   // **新增：用于显示聊天记录**
#include <QLineEdit>   // **新增：用于输入消息**
#include <QPushButton> // **新增：用于发送消息按钮**
#include <QTcpSocket>  // **新增：QTcpSocket，用于共享连接**
#include <QJsonDocument> // **新增：用于处理 JSON 消息**
#include <QJsonObject>   // **新增：用于处理 JSON 消息**
#include <QDataStream>   // **新增：用于数据包分帧**
#include <QMessageBox>   // **新增：用于显示错误信息**
#include <QCloseEvent>   // **新增：用于处理窗口关闭事件**
#include <QDebug>        // **新增：用于调试输出**
#include <QListWidget>  // **新增：用于显示在线用户列表**
#include <QSplitter>

class MainWindow : public QWidget {
    Q_OBJECT // 启用 Qt 的元对象系统

public:
    // **修改构造函数：现在它需要接收 LoginWindow 传递过来的共享 QTcpSocket 指针**
    explicit MainWindow(QTcpSocket *socket, QWidget *parent = nullptr);
    ~MainWindow(); // 析构函数

protected:
    // **新增：重写 closeEvent，用于在主窗口关闭时处理断开连接或退出程序**
    void closeEvent(QCloseEvent *event) override;

private slots:
    // **新增：QTcpSocket 相关的槽函数**
    void onSocketReadyRead();             // 当有新数据可读时触发（接收聊天消息）
    void onSocketDisconnected();          // 当服务器断开连接时触发
    void onSocketErrorOccurred(QAbstractSocket::SocketError socketError); // 当套接字发生错误时触发

    // **新增：聊天界面按钮的槽函数**
    void onSendButtonClicked(); // 发送消息按钮点击时触发

private:
    QTextEdit *chatDisplay;    // 显示聊天内容的区域
    QLineEdit *messageInput;   // 消息输入框
    QPushButton *sendButton;   // 发送按钮

    QListWidget *userListWidget;
    // **新增：共享 QTcpSocket 成员变量**
    QTcpSocket *mainTcpSocket;

    // **新增：用于数据包分帧的变量**
    quint32 m_blockSize;

    // **新增：辅助函数，用于处理接收到的 JSON 消息**
    void processIncomingMessage(const QJsonObject& message);
    // **新增：辅助函数，用于将消息发送到服务器**
    void sendMessageToServer(const QString &msg);
};

#endif // MAINWINDOW_H