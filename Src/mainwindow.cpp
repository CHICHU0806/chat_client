//
// Created by 20852 on 25-7-22.
//
#include "mainwindow.h"
#include "networkmanager.h"
#include "personalmsgwindow.h"
#include <qboxlayout> // 用于布局
#include <QHostAddress> // 用于 QTcpSocket
#include <QJsonDocument> // 用于 JSON 处理
#include <QJsonArray>
#include <QJsonObject>   // 用于 JSON 处理
#include <QDateTime>     // 用于显示消息时间

// 构造函数
MainWindow::MainWindow(QTcpSocket *socket,const QString& username,const QString& account, QWidget *parent)
    : QWidget(parent),
      mainTcpSocket(socket), // **将传入的共享 socket 赋值给成员变量**
      currentUsername(username),
      currentAccount(account),     // 账号ID
      personalMsgWindow(nullptr) // 初始化为空指针
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

    // 创建个人消息按钮（圆形）
    personalMsgButton = new QPushButton(this);
    personalMsgButton->setFixedSize(35, 35); // 设置为圆形大小
    personalMsgButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #FFFFFF;"
        "    border: 2px solid #CCCCCC;"
        "    border-radius: 17px;" // 半径为宽度的一半，形成圆形
        "    color: #666666;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F0F0F0;"
        "    border-color: #999999;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #E0E0E0;"
        "}"
    );

    topBarLayout->addWidget(personalMsgButton); // 添加到左侧
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
        "QPushButton:pressed {"
        "    background-color: #455A64;"  // 更深的颜色
        "    border-radius: 4px;"
        "}"
    );

    // 创建添加好友按钮
    addFriendButton = new QPushButton("➕", this);
    addFriendButton->setStyleSheet(
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
        "QPushButton:pressed {"
        "    background-color: #455A64;"  // 更深的颜色
        "    border-radius: 4px;"
        "}"
    );

    topBarLayout->addWidget(addFriendButton);    // 添加好友按钮
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
        "    background-color: #6690A0;"
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
    // 新增：监听输入框内容变化，控制发送按钮状态
    connect(messageInput, &QLineEdit::textChanged, this, &MainWindow::onMessageInputChanged);
    //个人信息按钮
    connect(personalMsgButton, &QPushButton::clicked, this, &MainWindow::onPersonalMsgButtonClicked);
    // 连接添加好友按钮
    connect(addFriendButton, &QPushButton::clicked, this, &MainWindow::onAddFriendButtonClicked);

    // 初始化添加好友窗口为空指针
    addFriendWindow = nullptr;

    // 新增：设置发送按钮初始状态为禁用
    sendButton->setEnabled(false);

    // 初始化用户列表
    initializeUserList();

    // 初始化数据库
    if (!ChatDatabase::instance()->initDatabase(currentAccount)) {
        QMessageBox::warning(this, "数据库错误", "无法初始化聊天记录数据库");
    }

    //连接networkmanager的信号
    auto* network = NetworkManager::instance();
    connect(network, &NetworkManager::chatMessageReceived, this, &MainWindow::handleChatMessage);

    //自动加载公共聊天室的历史记录（默认选中的是公共聊天室）
    loadChatHistory("public", "PUBLIC");

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

//重写 showEvent 方法，在窗口显示时设置输入框焦点
void MainWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    messageInput->setFocus();  // 窗口显示时聚焦到输入框
}

// **槽函数：发送消息按钮点击时触发**
void MainWindow::onSendButtonClicked() {
    QString messageText = messageInput->text().trimmed(); // 获取并去除空白
    if (messageText.isEmpty()) {
        return; // 静默返回，不显示警告
    }

    if (mainTcpSocket->state() == QAbstractSocket::ConnectedState) {
        // 移除多余的参数，直接调用
        sendMessageToServer(messageText);
        messageInput->clear();
    } else {
        QMessageBox::critical(this, "网络错误", "未连接到服务器，无法发送消息！");
        messageInput->setEnabled(false);
        sendButton->setEnabled(false);
    }
}

//输入框内容变化时的槽函数
void MainWindow::onMessageInputChanged(const QString& text) {
    // 当输入框有内容时启用发送按钮，否则禁用
    sendButton->setEnabled(!text.trimmed().isEmpty());
}

// 个人信息按钮槽函数
void MainWindow::onPersonalMsgButtonClicked() {
    if (!personalMsgWindow) {
        // 传入当前用户的用户名和账号信息
        personalMsgWindow = new PersonalMsgWindow(currentUsername, currentAccount, this);
        // 连接用户信息更新信号
        connect(personalMsgWindow, &PersonalMsgWindow::userInfoUpdated,this, &MainWindow::onUserInfoUpdated);
    }

    personalMsgWindow->show();
    personalMsgWindow->raise(); // 确保窗口显示在最前面
    personalMsgWindow->activateWindow(); // 激活窗口
}

//个人信息更新
void MainWindow::onUserInfoUpdated(const QString& newUsername, const QString& account) {
    // 更新主窗口显示的用户名
    currentUsername = newUsername;

    qDebug() << "主窗口用户名已更新为：" << newUsername;
    // 如果有显示用户名的UI元素，在这里更新
    // 例如：userNameLabel->setText(newUsername);
}

//添加好友按钮
void MainWindow::onAddFriendButtonClicked() {
    if (!addFriendWindow) {
        addFriendWindow = new AddFriendWindow(currentAccount, this);
        connect(addFriendWindow, &AddFriendWindow::friendAdded,
                this, [this](const QString& friendAccount, const QString& friendUsername) {
                    qDebug() << "新增好友：" << friendUsername << "(" << friendAccount << ")";
                    // 这里可以添加更新好友列表的逻辑
                });
    }

    addFriendWindow->show();
    addFriendWindow->raise();
    addFriendWindow->activateWindow();
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
        return;
    }

    QString chatTarget = item->data(Qt::UserRole).toString();

    if (chatTarget == "PUBLIC") {
        setWindowTitle("聊天室 - 公共聊天");
        messageInput->setPlaceholderText("发送到公共聊天室...");
        // 加载公共聊天历史记录
        loadChatHistory("public", "PUBLIC");
        qDebug() << "切换到公共聊天并加载历史记录";
    } else {
        setWindowTitle("聊天室 - 与 " + chatTarget + " 私聊");
        messageInput->setPlaceholderText("发送给 " + chatTarget + "...");
        // 加载私聊历史记录
        loadChatHistory("private", chatTarget);
        qDebug() << "切换到与" << chatTarget << "的私聊并加载历史记录";
    }

    // 切换聊天对象后自动聚焦到输入框
    messageInput->setFocus();
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
    QString type = message["type"].toString();
    QString status = message["status"].toString();
    QString sender = message["sender"].toString();        // 账号
    QString username = message["username"].toString();    // 用户名
    QString content = message["content"].toString();
    QString timestamp = message["timestamp"].toString();

    // 调试输出
    qDebug() << "收到消息 - type:" << type << "status:" << status
             << "sender:" << sender << "username:" << username
             << "content:" << content;

    // 只处理聊天消息
    if (type != "chatMessage") {
        return;
    }

    // 处理时间戳格式（从ISO格式转换为显示格式）
    QString displayTime;
    if (!timestamp.isEmpty()) {
        QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
        if (dt.isValid()) {
            displayTime = dt.toString("hh:mm:ss");
        } else {
            displayTime = QDateTime::currentDateTime().toString("hh:mm:ss");
        }
    } else {
        displayTime = QDateTime::currentDateTime().toString("hh:mm:ss");
    }

    // 如果是自己发送的消息，不重复显示（因为在sendMessageToServer中已经显示了）
    if (sender == currentAccount) {
        return;
    }

    // 根据status处理不同类型的消息
    if (status == "broadcast") {
        // 处理公共聊天室消息
        handlePublicChatMessage(username, content, displayTime);
    } else {
        // 处理其他类型消息（如私聊等）
        qDebug() << "收到未处理的消息类型:" << status;
    }

    // 确保滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();
}

// 这里简化，只发送消息内容。服务器应根据连接识别发送者。
void MainWindow::sendMessageToServer(const QString &msg) {
    QJsonObject request;
    request["type"] = "chatMessage";
    request["account"] = currentAccount;
    request["content"] = msg;

    QString chatType;
    QString chatTarget;

    QListWidgetItem* currentItem = userListWidget->currentItem();
    if (currentItem && currentItem->data(Qt::UserRole).toString() == "PUBLIC") {
        request["messageType"] = "public";
        chatType = "public";
        chatTarget = "PUBLIC";
    } else if (currentItem) {
        request["messageType"] = "private";
        request["targetAccount"] = currentItem->data(Qt::UserRole).toString();
        chatType = "private";
        chatTarget = currentItem->data(Qt::UserRole).toString();
    } else {
        request["messageType"] = "public";
        chatType = "public";
        chatTarget = "PUBLIC";
    }

    NetworkManager::instance()->sendMessage(request);

    // 保存自己发送的消息到数据库
    saveChatMessage(chatType, chatTarget, currentAccount, currentUsername, msg, true);

    qInfo() << "已发送聊天消息：" << msg;

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    chatDisplay->append("<br>");
    chatDisplay->append(QString("<font color='blue'>[%1] <b>%2:</b> %3</font>")
                         .arg(timestamp)
                         .arg(currentUsername)
                         .arg(msg));

    // 确保滚动到底部 - 使用多种方法确保生效
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();
}

//处理公共聊天消息
void MainWindow::handlePublicChatMessage(const QString& username, const QString& content, const QString& timestamp) {
    QListWidgetItem* currentItem = userListWidget->currentItem();
    if (!currentItem || currentItem->data(Qt::UserRole).toString() != "PUBLIC") {
        // 即使不在当前窗口也要保存消息
        saveChatMessage("public", "PUBLIC", "", username, content, false);
        qDebug() << "当前不在公共聊天室，消息已保存但不显示";
        return;
    }

    // 保存消息到数据库
    saveChatMessage("public", "PUBLIC", "", username, content, false);

    QString displayName = username.isEmpty() ? "匿名用户" : username;
    chatDisplay->append("<br>");
    chatDisplay->append(QString("<font color='green'>[%1] <b>%2:</b> %3</font>")
                         .arg(timestamp)
                         .arg(displayName)
                         .arg(content));

    // 确保滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();

    qDebug() << "显示并保存公共聊天消息 -" << displayName << ":" << content;
}

//聊天记录加载
void MainWindow::loadChatHistory(const QString& chatType, const QString& chatTarget) {
    // 限制加载最近30条记录
    QList<ChatMessage> messages = ChatDatabase::instance()->getRecentMessages(chatType, chatTarget, 30);

    chatDisplay->clear();

    if (chatType == "public") {
        chatDisplay->append("<font color='green'>--- 欢迎来到公共聊天室 ---</font>");
    } else {
        chatDisplay->append(QString("<font color='blue'>--- 与 %1 的私聊 ---</font>").arg(chatTarget));
    }

    // 检查是否还有更多历史记录
    QList<ChatMessage> allMessages = ChatDatabase::instance()->getMessages(chatType, chatTarget, 1000);
    bool hasMoreHistory = allMessages.size() > 30;

    if (hasMoreHistory) {
        chatDisplay->append("<font color='#0066CC'><u>点击这里加载更多历史记录...</u></font>");
    }

    // 显示历史消息
    for (const ChatMessage& msg : messages) {
        QString displayTime = msg.timestamp.toString("MM-dd hh:mm:ss");
        QString color = msg.isSelf ? "blue" : "green";

        chatDisplay->append("<br>");
        chatDisplay->append(QString("<font color='%1'>[%2] <b>%3:</b> %4</font>")
                             .arg(color)
                             .arg(displayTime)
                             .arg(msg.senderUsername)
                             .arg(msg.content));
    }

    // 确保滚动到底部（显示最新消息）
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();

    qDebug() << "已加载" << messages.size() << "条历史记录，总共有" << allMessages.size() << "条记录";
}

// 保存聊天消息到数据库
void MainWindow::saveChatMessage(const QString& chatType, const QString& chatTarget,
                                const QString& senderAccount, const QString& senderUsername,
                                const QString& content, bool isSelf) {
    ChatMessage message;
    message.chatType = chatType;
    message.chatTarget = chatTarget;
    message.senderAccount = senderAccount;
    message.senderUsername = senderUsername;
    message.content = content;
    message.timestamp = QDateTime::currentDateTime();
    message.isSelf = isSelf;

    if (!ChatDatabase::instance()->saveMessage(message)) {
        qWarning() << "保存聊天消息失败";
    }
}