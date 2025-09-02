//
// Created by 20852 on 25-7-22.
//
// mainwindow.h (客户端项目)

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatdatabase.h"
#include "addfriendwindow.h"
#include <QWidget>
#include <QLabel>      // 欢迎标签
#include <QTextEdit>   // 用于显示聊天
#include <QLineEdit>   // 用于输入消息
#include <QPushButton> // 用于发送消息按钮
#include <QTcpSocket>  // QTcpSocket，用于共享连接
#include <QJsonDocument> // 用于处理 JSON 消息
#include <QJsonObject>   // 用于处理 JSON 消息
#include <QCloseEvent>   // 用于处理窗口关闭事件
#include <QListWidget>  // 用于显示在线用户列表
#include <QSplitter>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>  // 添加阴影效果头文件
#include <QGraphicsEffect>             // 添加图形效果基类头文件
#include "chatdatabase.h"
#include "FriendListWindow.h"
#include <QProgressDialog> // 文件传输进度对话框
#include <QStandardPaths>  // 文件路径
#include <QDir>            // 目录操作
#include <QFile>           // 文件操作
#include <QFileInfo>       // 文件信息
#include <QUuid>           // UUID生成
#include <QCoreApplication> // 事件处理

// 文件传输信息结构体
struct FileTransferInfo {
    QString transferId;
    QString fileName;
    qint64 fileSize;
    qint64 bytesReceived;
    QString senderAccount;
    QString senderUsername;
    QString chatType;
    QString chatTarget;
    QFile* file;
    bool isComplete;
    QByteArray fileData;  // 存储文件数据
};

class PersonalMsgWindow; //声明

class MainWindow : public QWidget {
    Q_OBJECT // 启用 Qt 的元对象系统
public:
    // 修改构造函数：现在它需要接收 LoginWindow 传递过来的共享 QTcpSocket 指针
    explicit MainWindow(QTcpSocket *socket, const QString& username = QString(), const QString& account = QString(), QWidget *parent = nullptr);
    ~MainWindow(); // 析构函数

protected:
    //重写原生标题栏
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    //重写 closeEvent，用于在主窗口关闭时处理断开连接或退出程序
    void closeEvent(QCloseEvent *event) override;
    //重写 showEvent，用��开屏聚焦输入框
    void showEvent(QShowEvent* event) override;
    //重写双击标题栏最大化/还原窗口
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    // 添加事件过滤器来处理聊天区域的鼠标光标
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // NetworkManager 相关的槽函数
    void handleChatMessage(const QJsonObject& message);

    //聊天界面按钮的槽函数
    void onSendButtonClicked(); // 发送消息按钮点击时触发
    void onUserListItemClicked(QListWidgetItem* item); // 添加用户列表点击事件
    void onMessageInputChanged(const QString& text); // 输入框内容变化时触发

    //顶部栏个人信息设置
    void onPersonalMsgButtonClicked(); // 添加按钮点击槽函数=
    //用户个人信息处理
    void onUserInfoUpdated(const QString& newUsername, const QString& account);

    void onAddFriendButtonClicked();     // 新增：添加好友按钮槽函数

    void onFriendListButtonClicked();    // 好友列表按钮槽函数

    //离线消息处理
    void handleOfflineMessages(const QJsonObject& response);

    //接收好友列表
    void onFriendListReceived(const QJsonObject& response);

    //ai相关方法
    void onAiAnswerReceived(const QJsonObject& resp);

    // 文件传输相关槽函数
    void onSendFileButtonClicked();      // 发送文件按钮点击
    void onSendImageButtonClicked();     // 发送图片按钮点击
    void onFileTransferError(const QString& transferId, const QString& errorMsg); // 文件传输错误
    void onFileTransferProgress(const QString& transferId, qint64 bytesReceived, qint64 totalSize); // 文件传输进度
    void onFileTransferCompleted(const QString& transferId, const QString& filePath); // 文件传输完成
    void onFileTransferStarted(const QString& transferId, const QString& fileName, qint64 fileSize, const QString& senderUsername); // 文件传输开始

    // 新增：处理NetworkManager转发的文件传输信号
    void handleFileTransferResponse(const QJsonObject& response);
    void handleFileChunkReceived(const QByteArray& data);
    void handleUnknownMessage(const QJsonObject& obj); // 新增：处理未知消息槽函数

private:
    QPoint dragPosition;
    bool dragging = false;

    // 添加阴影和布局相关的成员变量
    QGraphicsDropShadowEffect *shadowEffect;
    QWidget *mainContainer;
    QVBoxLayout *containerLayout;

    QTextEdit *chatDisplay;    // 显示聊天内容的区域
    QLineEdit *messageInput;   // 消息输入框
    QPushButton *sendButton;   // 发送按钮

    QListWidget *userListWidget;

    // 共享 QTcpSocket 成员变量
    QTcpSocket *mainTcpSocket;

    // 当前登录用户名
    QString currentUsername;
    QString currentAccount;   // 账号ID（数据库中的唯一标识）

    //顶部栏个人信息设置
    QPushButton *personalMsgButton; // 添加按钮成员变量
    PersonalMsgWindow *personalMsgWindow; // 添加窗口成员变量

    QPushButton *addFriendButton;       // 添加好友按钮
    AddFriendWindow *addFriendWindow;   // 添加好友窗口
    QPushButton *friendListButton;      // 好友列表按钮
    FriendListWindow *friendListWindow;    // 好友列表窗口

    // 聊天记录加载相关
    QString currentChatType;     // 当前聊天类型
    QString currentChatTarget;   // 当前聊天目标
    QString formatMessage(const ChatMessage& msg);
    int loadedMessageCount;      // 已加载的消息数量
    bool isLoadingHistory;       // 是否正在加载历史记录

    bool isOfflineMessagesProcessed;  // 离线消息是否已处理完成

    QJsonArray currentFriendListJsonArray;// 当前好友列表数据

    //ai相关组件
    QLineEdit *aiInput;        // AI输入框
    QPushButton *aiAskButton;  // AI按钮
    QListWidgetItem* aiListItem = nullptr;

    // 文件传输相关成员变量
    QString downloadDir;  // 下载目录
    QMap<QString, FileTransferInfo> activeTransfers;  // 活跃的文件传输
    QMap<QString, QProgressDialog*> fileTransferDialogs;  // 文件传输进度对话框

    //用户列表相关函数
    void initializeUserList(); // 初始化用户列表
    void updateUserList(const QJsonArray& users); // 更新在线用户列表
    void requestUserList(); // 请求用户列表

    // 辅助函数
    void handlePublicChatMessage(const QString& senderAccount, const QString& senderUsername, const QString& content, const QString& timestamp);
    void handlePrivateChatMessage(const QString& senderAccount, const QString& senderUsername, const QString& content, const QString& timestamp);//私聊

    // 文件传输相关方法
    void initializeDownloadDir();  // 初始化下载目录
    void sendFileTransfer(const QString& filePath, const QString& chatType, const QString& chatTarget);  // 发送文件
    void processFileChunk(const QByteArray& data);  // 处理文件块
    void processFileTransfer(const QJsonObject& response);  // 处理文件传输响应
    void sendFileTransferAck(const QString& transferId, const QString& status, const QString& message = QString());  // 发送确认
    static void sendFileTransferProgress(const QString& transferId, qint64 bytesReceived, qint64 totalSize);  // 发送进度
    static QString formatFileSize(qint64 bytes);  // 格式化文件大小

    void sendMessageToServer(const QString &msg);

    // 聊天记录相关方法
    void loadChatHistory(const QString& chatType, const QString& chatTarget);
    void saveChatMessage(const QString& chatType, const QString& chatTarget, const QString& senderAccount, const QString& senderUsername, const QString& content, bool isSelf = false);

    // 滚动条相关方法
    void setupScrollBarConnection();
    void loadMoreHistory();
    bool hasMoreHistory() const;
    void displayMessages(const QList<ChatMessage>& messages);
    void scrollToBottom();

    void requestOfflineMessages();  // 请求离线消息
    void handleOfflinePublicMessageDirect(const QString& senderAccount, const QString& senderUsername,
                                    const QString& content, const QString& timestamp);
    void handleOfflinePrivateMessageDirect(const QString& senderAccount, const QString& senderUsername,
                                         const QString& content, const QString& timestamp);

    //好友列表相关方法
    void requestFriendList();           // 请求好友列表
    void updateFriendListUI(const QJsonArray& friends);  // 更新好友列表UI
    void addFriendToList(const QJsonObject& friendObj, bool isOnline); // 添加单个好友到列表

    //窗口相关方法
    void restoreNormalWindowStyle();
    void setMaximizedWindowStyle();
};

#endif // MAINWINDOW_H

