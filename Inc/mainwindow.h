//
// Created by 20852 on 25-7-22.
//
// mainwindow.h (客户端项目)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>      // 欢迎标签
#include <QTextEdit>   // 用于显示聊天记录
#include <QLineEdit>   // 用于输入消息
#include <QPushButton> // 用于发送消息按钮
#include <QTcpSocket>  // QTcpSocket，用于共享连接
#include <QJsonDocument> // 用于处理 JSON 消息
#include <QJsonObject>   // 用于处理 JSON 消息
#include <QCloseEvent>   // 用于处理窗口关闭事件
#include <QListWidget>  // 用于显示在线用户列表
#include <QSplitter>
#include <QMessageBox>

class MainWindow : public QWidget {
    Q_OBJECT // 启用 Qt 的元对象系统

public:
    // 修改构造函数：现在它需要接收 LoginWindow 传递过来的共享 QTcpSocket 指针
    explicit MainWindow(QTcpSocket *socket, const QString& username = QString(), QWidget *parent = nullptr);
    ~MainWindow(); // 析构函数

protected:
    //重写 closeEvent，用于在主窗口关闭时处理断开连接或退出程序
    void closeEvent(QCloseEvent *event) override;

private slots:
    // NetworkManager 相关的槽函数
    void handleChatMessage(const QJsonObject& message);

    //聊天界面按钮的槽函数
    void onSendButtonClicked(); // 发送消息按钮点击时触发
    void onUserListItemClicked(QListWidgetItem* item); // 添加用户列表点击事件
    void onMessageInputChanged(const QString& text); // 输入框内容变化时触发

private:
    QTextEdit *chatDisplay;    // 显示聊天内容的区域
    QLineEdit *messageInput;   // 消息输入框
    QPushButton *sendButton;   // 发送按钮

    QListWidget *userListWidget;

    // 共享 QTcpSocket 成员变量
    QTcpSocket *mainTcpSocket;

    // 当前登录用户名
    QString currentUsername;

    //用户列表相关函数
    void initializeUserList(); // 初始化用户列表
    void updateUserList(const QJsonArray& users); // 更新在线用户列表
    void requestUserList(); // 请求用户列表

    // 辅助函数
    void handlePublicChatMessage(const QString& username, const QString& content, const QString& timestamp);//处理公共聊天消息
     void sendMessageToServer(const QString &msg, const QString &account = QString());
};

#endif // MAINWINDOW_H