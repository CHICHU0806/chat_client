//
// Created by 20852 on 25-7-22.
//
#include "mainwindow.h"
#include "networkmanager.h"
#include <qboxlayout> // 用于布局
#include <QHostAddress> // 用于 QTcpSocket
#include <QJsonDocument> // 用于 JSON 处理
#include <QJsonArray>
#include <QJsonObject>   // 用于 JSON 处理
#include <QDateTime>     // 用于显示消息时间

// 构造函数
MainWindow::MainWindow(QTcpSocket *socket, QWidget *parent)
    : QWidget(parent),
      mainTcpSocket(socket) // **将传入的共享 socket 赋值给成员变量**
{
    setWindowTitle(" "); // 设置窗口标题
    setMinimumSize(1000, 750);        // 设置窗口最小大小

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 创建顶部状态栏
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(50);  // 固定高度
    topBar->setStyleSheet(
        "QWidget {"
        "    background-color: #6690A0;"  // 深色背景
        "    border-bottom: 1px solid #1E1E1E;"  // 底部边框
        "}"
    );

    // 顶部状态栏的布局
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);

    topBarLayout->addStretch();  // 左侧弹性空间

    // 添加一些示例控件到顶部栏
    QPushButton *settingsButton = new QPushButton("⚙", this);
    settingsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    padding: 5px 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #607D8B;"
        "    border-radius: 4px;"  // hover 状态也需要添加圆角
        "}"
    );

    // 添加弹性空间和按钮
    topBarLayout->addWidget(settingsButton);

    // 创建主分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setHandleWidth(1);  // 设置分割条的宽度
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #CCCCCC;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #999999;"
        "}"
    );

    // 1. 左侧用户列表区域
    QWidget *leftWidget = new QWidget(this);
    leftWidget->setMinimumWidth(180);  // 添加这行：设置最小宽度
    leftWidget->setMaximumWidth(350);  // 添加这行：设置最大宽度
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    userListWidget = new QListWidget(leftWidget);
    userListWidget->setStyleSheet(
        "QListWidget {"
        "    border: none;"
        "    background-color: white;"
        "}"
    );

    leftLayout->addWidget(userListWidget);

    // 2. 中间聊天区域
    QWidget *centerWidget = new QWidget(this);
    centerWidget->setMinimumWidth(400);  // 添加这行：设置聊天区域最小宽度
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    chatDisplay = new QTextEdit(centerWidget);
    chatDisplay->setReadOnly(true);
    chatDisplay->setStyleSheet(
        "QTextEdit {"
        "    border: none;"
        "    background-color: white;"
        "}"
    );

    // 创建输入区域
    QWidget *inputArea = new QWidget(centerWidget);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(5, 5, 5, 5);

    messageInput = new QLineEdit(inputArea);
    messageInput->setPlaceholderText("输入消息...");
    messageInput->setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #CCCCCC;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "}"
    );

    sendButton = new QPushButton("发送", inputArea);
    sendButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #1E90FF;"
        "    color: white;"
        "    border: none;"
        "    padding: 5px 15px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1873CC;"
        "}"
    );

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(mainSplitter);

    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);

    centerLayout->addWidget(chatDisplay);
    centerLayout->addWidget(inputArea);

    // 添加部件到分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(centerWidget);

    // ← 在这里添加分割器约束设置
    mainSplitter->setCollapsible(0, false);
    mainSplitter->setCollapsible(1, false);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);

    // 设置初始大小比例
    QList<int> sizes;
    sizes << 200 << 800;  // 左侧固定200像素，右侧占据剩余空间
    mainSplitter->setSizes(sizes);

    // 将分割器添加到主布局
    mainLayout->addWidget(mainSplitter);

    // 连接信号与槽
    // 连接发送按钮的 clicked 信号到 onSendMessageButtonClicked 槽
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    // 允许按 Enter 键发送消息
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendButtonClicked);
    // 连接用户列表点击事件
    connect(userListWidget, &QListWidget::itemClicked, this, &MainWindow::onUserListItemClicked);

    // 初始化用户列表
    initializeUserList();

    //连接networkmanager的信号
    auto* network = NetworkManager::instance();
    connect(network, &NetworkManager::chatMessageReceived, this, &MainWindow::handleChatMessage);

    qDebug() << "MainWindow initialized with shared socket.";
}

// 析构函数
MainWindow::~MainWindow() {
    qDebug() << "MainWindow destroyed.";
    // m_tcpSocket 不需要在这里 delete，因为它是由 LoginWindow 管理的共享指针
}

// 重写 closeEvent 方法，在窗口关闭事件发生时断开socket连接
void MainWindow::closeEvent(QCloseEvent *event) {
    // 可以在这里做一些清理工作，例如，如果主窗口关闭意味着整个客户端退出，则可以断开连接
    // 但如果应用程序可能还有其他部分，则不在此处断开
    // 为了简单起见，我们在这里断开连接，表示客户端应用程序的结束
    if (mainTcpSocket->state() == QAbstractSocket::ConnectedState) {
        mainTcpSocket->disconnectFromHost();
        qInfo() << "MainWindow closed, socket disconnected.";
    }
    QWidget::closeEvent(event); // 调用基类的 closeEvent，确保窗口正常关闭
}

// **槽函数：发送消息按钮点击时触发**
void MainWindow::onSendButtonClicked() {
    QString messageText = messageInput->text().trimmed(); // 获取并去除空白
    if (messageText.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "消息内容不能为空！");
        return;
    }

    if (mainTcpSocket->state() == QAbstractSocket::ConnectedState) {
        // 假设我们有一个默认的发送者，或者从登录信息中获取
        // 为了简单，暂时用 "You" 或获取当前登录的用户名 (如果 LoginWindow 存储了)
        // 例如：QString currentUsername = "TestUser"; // 真实项目中应从登录信息获取
        // sendMessageToServer(messageText, currentUsername);

        // 为了简化，我们只发送消息文本，服务器可以根据连接的socket识别发送者
        sendMessageToServer(messageText);

        messageInput->clear(); // 清空输入框
    } else {
        QMessageBox::critical(this, "网络错误", "未连接到服务器，无法发送消息！");
        messageInput->setEnabled(false);
        sendButton->setEnabled(false);
    }
}

// 初始化用户列表
void MainWindow::initializeUserList() {
    userListWidget->clear();

    // 设置 QListWidget 的样式
    userListWidget->setStyleSheet(
        "QListWidget {"
        "    border: none;"
        "    background-color: #6690A0;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    height: 45px;"                    // 固定每项高度
        "    padding: 8px 12px;"              // 内边距
        "    margin: 2px 8px;"                // 外边距
        "    border-radius: 6px;"             // 圆角
        "    background-color: #6690A0;"        // 默认背景色
        "    border: 1px solid #e0e0e0;"      // 边框
        "}"
        "QListWidget::item:hover {"
        "    background-color: #607D8B;"      // hover时的背景色（浅蓝色）
        "    border-color: white;"          // hover时的边框色
        "}"
        "QListWidget::item:selected {"
        "    background-color: #009688;"      // 选中时的背景色（蓝色）
        "    color: white;"                   // 选中时的文字颜色
        "    border-color: #1976d2;"          // 选中时的边框色
        "}"
        "QListWidget::item:selected:hover {"
        "    background-color: #009688;"      // 选中且hover时的背景色（深蓝色）
        "}"
    );

    // 添加公共聊天室选项
    QListWidgetItem* publicItem = new QListWidgetItem("📢 公共聊天室");
    publicItem->setData(Qt::UserRole, "PUBLIC"); // 存储聊天类型标识
    // 设置字体但不设置颜色，让CSS样式控制
    QFont publicFont;
    publicFont.setPointSize(11);
    publicFont.setBold(true);
    publicItem->setFont(publicFont);

    userListWidget->addItem(publicItem);

    // 添加分隔线样式的文本项
    QListWidgetItem* separator = new QListWidgetItem("────── 在线用户 ──────");
    separator->setFlags(Qt::NoItemFlags); // 不可选中
    separator->setTextAlignment(Qt::AlignCenter);
    // 给分隔线设置特殊样式
    QFont separatorFont;
    separatorFont.setPointSize(9);
    separator->setFont(separatorFont);
    separator->setForeground(QColor(128, 128, 128));
    // 设置默认选中公共聊天室
    userListWidget->setCurrentItem(publicItem);

    // 模拟添加一些在线用户（后续从服务器获取）
    QStringList mockUsers = {"用户1", "用户2", "用户3"};
    for (const QString& username : mockUsers) {
        QListWidgetItem* item = new QListWidgetItem("👤 " + username);
        item->setData(Qt::UserRole, username);

        // 设置普通用户的字体
        QFont userFont;
        userFont.setPointSize(10);
        item->setFont(userFont);

        userListWidget->addItem(item);
    }

    qDebug() << "用户列表已初始化";
}

// 处理用户列表点击事件
void MainWindow::onUserListItemClicked(QListWidgetItem* item) {
    if (!item || item->flags() == Qt::NoItemFlags) {
        return; // 忽略分隔线点击
    }

    QString chatTarget = item->data(Qt::UserRole).toString();

    if (chatTarget == "PUBLIC") {
        // 切换到公共聊天
        setWindowTitle("聊天室 - 公共聊天");
        messageInput->setPlaceholderText("发送到公共聊天室...");
        chatDisplay->clear();
        chatDisplay->append("<font color='green'>--- 欢迎来到公共聊天室 ---</font>");
        qDebug() << "切换到公共聊天";
    } else {
        // 切换到私聊
        setWindowTitle("聊天室 - 与 " + chatTarget + " 私聊");
        messageInput->setPlaceholderText("发送给 " + chatTarget + "...");
        chatDisplay->clear();
        chatDisplay->append(QString("<font color='blue'>--- 与 %1 的私聊 ---</font>").arg(chatTarget));
        qDebug() << "切换到与" << chatTarget << "的私聊";
    }
}

// 更新用户列表（从服务器接收到用户列表时调用）
void MainWindow::updateUserList(const QJsonArray& users) {
    // 清除除了公共聊天室和分隔线之外的所有项
    for (int i = userListWidget->count() - 1; i >= 2; i--) {
        delete userListWidget->takeItem(i);
    }

    // 添加在线用户
    for (const auto& userValue : users) {
        QString username = userValue.toString();
        if (username != currentUsername) { // 不显示自己
            QListWidgetItem* item = new QListWidgetItem("👤 " + username);
            item->setData(Qt::UserRole, username);
            // 设置普通用户的字体
            QFont userFont;
            userFont.setPointSize(10);
            item->setFont(userFont);
            userListWidget->addItem(item);
        }
    }

    qDebug() << "用户列表已更新，在线用户数：" << users.size();
}

// 请求用户列表
void MainWindow::requestUserList() {
    QJsonObject request;
    request["type"] = "getUserList";

    NetworkManager::instance()->sendMessage(request);
    qDebug() << "已请求用户列表";
}

// 处理接收到的聊天消息
void MainWindow::handleChatMessage(const QJsonObject& message) {
    QString sender = message["sender"].toString();
    QString content = message["content"].toString();
    QString timestamp = message["timestamp"].toString();

    // 暂时简单显示所有消息
    chatDisplay->append(QString("[%1] <b>%2:</b> %3")
                         .arg(timestamp)
                         .arg(sender)
                         .arg(content));

    // 滚动到底部
    chatDisplay->moveCursor(QTextCursor::End);
}

// 这里简化，只发送消息内容。服务器应根据连接识别发送者。
void MainWindow::sendMessageToServer(const QString &msg) {
    QJsonObject request;
    request["type"] = "chatMessage"; // 请求类型：聊天消息
    // request["sender"] = "YourUsername"; // 实际项目中应从登录的用户信息中获取
    request["content"] = msg;

    QByteArray jsonData = QJsonDocument(request).toJson(QJsonDocument::Compact);

    QByteArray dataBlock;
    QDataStream out(&dataBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << (quint32)0; // 预留总长度位置
    out << jsonData;   // 写入实际 JSON 数据

    out.device()->seek(0);
    out << (quint32)(dataBlock.size() - sizeof(quint32));

    mainTcpSocket->write(dataBlock);
    mainTcpSocket->flush(); // 强制发送缓冲数据

    qInfo() << "已发送聊天消息：" << msg;

    // 假设自己发送的消息也显示在自己的聊天框中
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    chatDisplay->append(QString("<font color='blue'>[%1] <b>我:</b> %2</font>")
                         .arg(timestamp)
                         .arg(msg));
}