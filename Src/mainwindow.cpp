//
// Created by 20852 on 25-7-22.
//
#include "mainwindow.h"
#include "networkmanager.h"
#include "personalmsgwindow.h"
#include "FriendListWindow.h"
#include <qboxlayout> // 用于布局
#include <QHostAddress> // 用于 QTcpSocket
#include <QJsonArray>
#include <QJsonObject>   // 用于 JSON 处理
#include <QDateTime>     // 用于显示消息时间
#include <QScrollBar>   // 用于滚动条控制
#include <QTimer>

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

    // 创建好友列表按钮
    friendListButton = new QPushButton("👥", this);
    friendListButton->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    padding: 5px 15px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #607D8B;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #455A64;"
        "    border-radius: 4px;"
        "}"
    );

    topBarLayout->addWidget(friendListButton);   // 添加好友列表按钮

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
    chatDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse);
    chatDisplay->setStyleSheet(
        "QTextEdit {"
        "    border: none;"
        "    background-color: #6690A0;"
        "    padding: 10px;"
        "}"
    );

    setupScrollBarConnection();

    // 初始化聊天记录相关变量
    currentChatType = "public";
    currentChatTarget = "PUBLIC";
    loadedMessageCount = 0;
    isLoadingHistory = false;

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
    // 连接好友列表按钮
    connect(friendListButton, &QPushButton::clicked, this, &MainWindow::onFriendListButtonClicked);
    // 连接好友添加信号
    connect(addFriendWindow, &AddFriendWindow::friendAdded, this, [this](const QString& account, const QString& username) {Q_UNUSED(account)Q_UNUSED(username)
    // 刷新好友列表
    QTimer::singleShot(1000, this, &MainWindow::requestFriendList);});

    // 初始化添加好友窗口为空指针
    addFriendWindow = nullptr;

    // 初始化：
    friendListWindow = nullptr;

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
    connect(network, &NetworkManager::offlineMessagesReceived, this, &MainWindow::handleOfflineMessages);
    connect(network, &NetworkManager::friendListReceived, this, &MainWindow::onFriendListReceived);


    QTimer::singleShot(500, this, [this]() {
        requestFriendList();
    });

    QTimer::singleShot(500, this, [this]() {
        requestOfflineMessages();
    });

    isOfflineMessagesProcessed = false;

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
                    qDebug() << "已发送申请：" << friendUsername << "(" << friendAccount << ")";
                    // 这里可以添加更新好友列表的逻辑
                });
    }

    addFriendWindow->show();
    addFriendWindow->raise();
    addFriendWindow->activateWindow();
}

// 好友列表按钮
void MainWindow::onFriendListButtonClicked() {
    if (!friendListWindow) {
        friendListWindow = new FriendListWindow(currentAccount, this);
        connect(friendListWindow, &FriendListWindow::friendSelected,
                this, [this](const QString& friendAccount, const QString& friendUsername) {
                    qDebug() << "选择好友进行聊天：" << friendUsername << "(" << friendAccount << ")";
                    // 这里可以添加切换到私聊的逻辑
                });
    }

    friendListWindow->show();
    friendListWindow->raise();
    friendListWindow->activateWindow();
}

//离线消息处理
void MainWindow::handleOfflineMessages(const QJsonObject& response) {
    qDebug() << "=== 开始处理离线消息响应 ===";
    qDebug() << "收到离线消息响应:" << response;

    // 注意：实际的离线消息是通过 chatMessage 类型的消息推送的
    // 这个函数主要用于标记离线消息处理完成和触发界面刷新

    isOfflineMessagesProcessed = true;

    qDebug() << "离线消息处理完成，重新加载聊天历史";
    loadChatHistory("public", "PUBLIC");

    qDebug() << "=== 离线消息处理结束 ===";
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

    qDebug() << "用户列表已初始化";
}

// 处理用户列表点击事件
void MainWindow::onUserListItemClicked(QListWidgetItem* item) {
    if (!item || item->flags() == Qt::NoItemFlags) {
        return; // 跳过分隔线等不可选择项
    }

    QString chatTarget = item->data(Qt::UserRole).toString();

    if (chatTarget == "PUBLIC") {
        // 切换到公共聊天
        currentChatType = "public";
        currentChatTarget = "PUBLIC";
        chatDisplay->clear();
        loadChatHistory(currentChatType, currentChatTarget);

        // 更新窗口标题
        setWindowTitle(QString("聊天室 - %1 - 公共聊天").arg(currentUsername));
    }
    else if (chatTarget == "SEPARATOR") {
        // 分隔线，不做任何操作
        return;
    }
    else {
        // 切换到好友私聊
        QString friendUsername = item->data(Qt::UserRole + 1).toString();
        bool isOnline = item->data(Qt::UserRole + 2).toBool();

        currentChatType = "private";
        currentChatTarget = chatTarget; // 好友账号
        chatDisplay->clear();
        loadChatHistory(currentChatType, currentChatTarget);

        // 更新窗口标题
        QString onlineStatus = isOnline ? "[在线]" : "[离线]";
        setWindowTitle(QString("聊天室 - %1 - 与 %2 的私聊 %3")
                      .arg(currentUsername, friendUsername, onlineStatus));

        qDebug() << "切换到好友私聊:" << friendUsername << "(" << chatTarget << ")";
    }

    // 聚焦输入框
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
    qDebug() << "=== handleChatMessage 调试信息 ===";
    qDebug() << "当前用户账号:" << currentAccount;
    qDebug() << "当前用户名:" << currentUsername;
    qDebug() << "消息发送者账号:" << message["sender"].toString();
    qDebug() << "消息发送者用户名:" << message["username"].toString();
    qDebug() << "消息内容:" << message["content"].toString();
    qDebug() << "消息类型:" << message["type"].toString();
    qDebug() << "消息状态:" << message["status"].toString();

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
        qDebug() << "不是聊天消息，忽略";
        return;
    }

    // 关键调试：检查这里的判断
    qDebug() << "sender == currentAccount?" << (sender == currentAccount);
    if (sender == currentAccount) {
        qDebug() << "是自己发送的消息，跳过显示";
        return;
    }

    // 处理离线消息：不跳过自己的消息，因为离线消息需要保存和显示
    if (status == "offline_broadcast" || status == "offline_private") {
        qDebug() << "处理离线消息，状态:" << status;

        // 离线消息直接保存到数据库，不立即显示
        ChatMessage msg;
        msg.senderAccount = sender;
        msg.senderUsername = username;
        msg.content = content;
        msg.timestamp = QDateTime::fromString(timestamp, Qt::ISODate);
        msg.isSelf = (sender == currentAccount);

        if (status == "offline_broadcast") {
            msg.chatType = "public";
            msg.chatTarget = "PUBLIC";
        } else if (status == "offline_private") {
            msg.chatType = "private";
            msg.chatTarget = message["target"].toString();
        }

        bool saved = ChatDatabase::instance()->saveMessage(msg);
        qDebug() << "保存离线消息结果:" << saved << "内容:" << content.left(20);

        return; // 离线消息不立即显示，等待统一加载
    }

    qDebug() << "准备显示其他人的消息";

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

    // 根据status处理不同类型的消息
    if (status == "broadcast") {
        handlePublicChatMessage(username, content, displayTime);
        // 保存到数据库
        saveChatMessage("public", "PUBLIC", sender, username, content, false);
    } else if (status == "private") {
        handlePrivateChatMessage(username, content, displayTime);
        // 保存到数据库
        saveChatMessage("private", message["target"].toString(), sender, username, content, false);
    }

    // 确保滚动到底部
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();
}

// 这里简化，只发送消息内容。服务器应根据连接识别发送者。
void MainWindow::sendMessageToServer(const QString &msg) {
    if (msg.trimmed().isEmpty()) {
        return;
    }

    QJsonObject message;
    message["account"] = currentAccount;
    message["username"] = currentUsername;
    message["content"] = msg;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 根据当前聊天类型设置消息类型和目标
    if (currentChatType == "public") {
        message["type"] = "chatMessage";
        message["chatType"] = "public";
        // 公共聊天不需要 targetAccount
    } else if (currentChatType == "private") {
        message["type"] = "privateChatMessage";  // 私聊使用不同的类型
        message["chatType"] = "private";
        message["targetAccount"] = currentChatTarget;  // 私聊需要目标账号
    }

    // 发送到服务器
    NetworkManager::instance()->sendMessage(message);

    // 立即显示自己的消息到聊天区域
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");

    // 使用table布局实现右对齐 - 自己的消息
    QString bubbleHtml = QString(
        "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
        "<tr><td style='text-align: right; vertical-align: top;'>"
        "<div style='display: inline-block; text-align: right; max-width: 60%;'>"
        "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
        "<div><span style='background-color: #1E90FF; color: white; border-radius: 18px; "
        "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
        "text-align: right; display: inline-block; white-space: pre-wrap; "
        "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
        "</div></td></tr></table>"
    ).arg(currentUsername, timestamp, msg.toHtmlEscaped());

    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(bubbleHtml);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();

    // 保存到数据库
    saveChatMessage(currentChatType, currentChatTarget, currentAccount, currentUsername, msg, true);

    // 清空输入框
    messageInput->clear();
    sendButton->setEnabled(false);
}

//处理公共聊天消息
void MainWindow::handlePublicChatMessage(const QString& username, const QString& content, const QString& timestamp) {
    QString displayName = (username == currentUsername) ? "我" : username;

    // 使用table布局实现左对齐 - 别人的消息
    QString bubbleHtml = QString(
        "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
        "<tr><td style='text-align: left; vertical-align: top;'>"
        "<div style='display: inline-block; text-align: left; max-width: 60%;'>"
        "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
        "<div><span style='background-color: #F0F0F0; color: #333; border-radius: 18px; "
        "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
        "text-align: left; display: inline-block; white-space: pre-wrap; "
        "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
        "</div></td></tr></table>"
    ).arg(displayName, timestamp, content.toHtmlEscaped());

    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(bubbleHtml);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();

    // 保存到数据库
    saveChatMessage("public", "PUBLIC", username, username, content, username == currentUsername);
}

//处理私聊消息
void MainWindow::handlePrivateChatMessage(const QString& username, const QString& content, const QString& timestamp) {
    QString displayName = username;

    // 使用与公共消息相同的左对齐气泡样式
    QString bubbleHtml = QString(
        "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
        "<tr><td style='text-align: left; vertical-align: top;'>"
        "<div style='display: inline-block; text-align: left; max-width: 60%;'>"
        "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
        "<div><span style='background-color: #F0F0F0; color: #333; border-radius: 18px; "
        "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
        "text-align: left; display: inline-block; white-space: pre-wrap; "
        "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
        "</div></td></tr></table>"
    ).arg(displayName, timestamp, content.toHtmlEscaped());

    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(bubbleHtml);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();

    saveChatMessage("private", "PRIVATE", username, username, content, username == currentUsername);//保存
}


//聊天记录加载
void MainWindow::loadChatHistory(const QString& chatType, const QString& chatTarget) {
        qDebug() << "=== loadChatHistory 开始 ===";
        qDebug() << "聊天类型:" << chatType << "聊天目标:" << chatTarget;

        // 更新当前聊天信息
        currentChatType = chatType;
        currentChatTarget = chatTarget;
        loadedMessageCount = 0;
        isLoadingHistory = false;

        // 清空聊天显示区域
        chatDisplay->clear();

        // 增加加载数量以确保包含所有离线消息
        QList<ChatMessage> messages = ChatDatabase::instance()->getRecentMessages(chatType, chatTarget, 100);

        loadedMessageCount = messages.size();

        qDebug() << "从本地数据库加载了" << messages.size() << "条历史记录";

        if (messages.isEmpty()) {
            qDebug() << "没有历史记录可显示";
            return;
        }

        // 直接在这里显示消息
        QTextCursor cursor = chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);

        for (const auto& message : messages) {
            QString bubbleHtml;
            QString timestamp = message.timestamp.toString("hh:mm:ss");

            if (message.isSelf) {
                // 自己的消息（右对齐）
                bubbleHtml = QString(
                    "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
                    "<tr><td style='text-align: right; vertical-align: top;'>"
                    "<div style='display: inline-block; text-align: right; max-width: 60%;'>"
                    "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
                    "<div><span style='background-color: #1E90FF; color: white; border-radius: 25px; "
                    "padding: 12px 20px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
                    "text-align: right; display: inline-block; white-space: pre-wrap; "
                    "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
                    "</div></td></tr></table>"
                ).arg(message.senderUsername, timestamp, message.content.toHtmlEscaped());
            } else {
                // 别人的消息（左对齐）
                bubbleHtml = QString(
                    "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
                    "<tr><td style='text-align: left; vertical-align: top;'>"
                    "<div style='display: inline-block; text-align: left; max-width: 60%;'>"
                    "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
                    "<div><span style='background-color: #F0F0F0; color: #333; border-radius: 18px; "
                    "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
                    "text-align: left; display: inline-block; white-space: pre-wrap; "
                    "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
                    "</div></td></tr></table>"
                ).arg(message.senderUsername, timestamp, message.content.toHtmlEscaped());
            }
            cursor.insertHtml(bubbleHtml);
        }

        // 设置光标到末尾并滚动
        chatDisplay->setTextCursor(cursor);
        chatDisplay->ensureCursorVisible();

        qDebug() << "聊天历史加载完成，共显示" << messages.size() << "条消息";
        qDebug() << "=== loadChatHistory 结束 ===";
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

// 检查是否有更多历史记录可加载
void MainWindow::setupScrollBarConnection() {
    QScrollBar* scrollBar = chatDisplay->verticalScrollBar();

    connect(scrollBar, &QScrollBar::valueChanged, this, [this](int value) {
        QScrollBar* bar = chatDisplay->verticalScrollBar();

        // 检查是否滚动到顶部且不在加载状态
        if (value == bar->minimum() && !isLoadingHistory && hasMoreHistory()) {
            loadMoreHistory();
        }
    });
}

// 检查是否有更多历史记录
void MainWindow::loadMoreHistory() {
    if (isLoadingHistory || !hasMoreHistory()) {
        return;
    }

    isLoadingHistory = true;

    // 获取当前文档内容高度
    QTextDocument* doc = chatDisplay->document();
    qreal oldHeight = doc->size().height();

    // 获取更多历史消息，使用 offset 分页
    QList<ChatMessage> moreMessages = ChatDatabase::instance()->getRecentMessages(
        currentChatType, currentChatTarget, 20, loadedMessageCount
    );

    if (!moreMessages.isEmpty()) {
        // 在文档开头插入新消息
        QTextCursor cursor(doc);
        cursor.movePosition(QTextCursor::Start);

        QString historyContent;
        for (const auto& msg : moreMessages) {
            historyContent += formatMessage(msg);
        }

        cursor.insertHtml(historyContent);

        // 更新加载计数
        loadedMessageCount += moreMessages.size();

        // 恢复滚动位置 - 基于内容高度变化
        qreal newHeight = doc->size().height();
        qreal heightDiff = newHeight - oldHeight;

        QScrollBar* scrollBar = chatDisplay->verticalScrollBar();
        int newPosition = scrollBar->value() + static_cast<int>(heightDiff);
        scrollBar->setValue(newPosition);
    }

    isLoadingHistory = false;
}

bool MainWindow::hasMoreHistory() const {
    // 检查数据库中是否还有更多消息
    // 如果当前已加载的消息数量小于数据库中的总消息数量，则还有更多历史记录
    int totalCount = ChatDatabase::instance()->getMessageCount(currentChatType, currentChatTarget);
    return loadedMessageCount < totalCount;
}

QString MainWindow::formatMessage(const ChatMessage& msg) {
    QString timeStr = msg.timestamp.toString("hh:mm:ss");
    QString displayName = msg.isSelf ? "我" : msg.senderUsername;

    if (msg.isSelf) {
        // 自己的消息 - 右对齐
        return QString(
            "<table width='100%' style='margin: 10px 0; border-collapse: collapse;'>"
            "<tr><td style='text-align: right; vertical-align: top;'>"
            "<div style='display: inline-block; text-align: right; max-width: 60%;'>"
            "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
            "<div><span style='background-color: #1E90FF; color: white; border-radius: 18px; "
            "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
            "text-align: left; display: inline-block; white-space: pre-wrap; "
            "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
            "</div></td></tr></table>"
        ).arg(displayName, timeStr, msg.content.toHtmlEscaped());
    } else {
        // 别人的消息 - 左对齐
        return QString(
            "<table width='100%' style='margin: 10px 0; border-collapse: collapse;'>"
            "<tr><td style='text-align: left; vertical-align: top;'>"
            "<div style='display: inline-block; text-align: left; max-width: 60%;'>"
            "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
            "<div><span style='background-color: #F0F0F0; color: #333; border-radius: 18px; "
            "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
            "text-align: left; display: inline-block; white-space: pre-wrap; "
            "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
            "</div></td></tr></table>"
        ).arg(displayName, timeStr, msg.content.toHtmlEscaped());
    }
}

void MainWindow::displayMessages(const QList<ChatMessage>& messages) {
    for (const auto& msg : messages) {
        QString formattedMsg = formatMessage(msg);
        QTextCursor cursor = chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertHtml(formattedMsg);
        chatDisplay->setTextCursor(cursor);
    }
}

void MainWindow::scrollToBottom() {
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
    chatDisplay->ensureCursorVisible();
}

// 请求离线消息
void MainWindow::requestOfflineMessages() {
    qDebug() << "=== 请求离线消息 ===";

    QJsonObject request;
    request["type"] = "getOfflineMessages";
    request["account"] = currentAccount;

    NetworkManager::instance()->sendMessage(request);
    qDebug() << "已发送离线消息请求，账号:" << currentAccount;

    // 添加超时机制：5秒后如果还没收到响应，就直接加载历史记录
    QTimer::singleShot(5000, this, [this]() {
        if (!isOfflineMessagesProcessed) {
            qDebug() << "离线消息请求超时，直接加载聊天历史";
            isOfflineMessagesProcessed = true;
            loadChatHistory("public", "PUBLIC");
        }
    });
}

void MainWindow::handleOfflinePublicMessageDirect(const QString& senderAccount,
                                                 const QString& senderUsername,
                                                 const QString& content,
                                                 const QString& timestamp) {
    // 直接保存到数据库，不立即显示（避免界面混乱）
    ChatMessage message;
    message.chatType = "public";
    message.chatTarget = "PUBLIC";
    message.senderAccount = senderAccount;
    message.senderUsername = senderUsername;
    message.content = content;

    // 使用服务器提供的时间戳
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    if (dt.isValid()) {
        message.timestamp = dt;
    } else {
        message.timestamp = QDateTime::currentDateTime();
    }

    message.isSelf = false;

    if (!ChatDatabase::instance()->saveMessage(message)) {
        qWarning() << "保存离线公共消息失败";
    } else {
        qDebug() << "离线公共消息已保存:" << content;
    }
}

void MainWindow::handleOfflinePrivateMessageDirect(const QString& senderAccount,
                                                  const QString& senderUsername,
                                                  const QString& content,
                                                  const QString& timestamp) {
    // 直接保存到数据库
    ChatMessage message;
    message.chatType = "private";
    message.chatTarget = senderAccount;  // 私聊以对方账号作为target
    message.senderAccount = senderAccount;
    message.senderUsername = senderUsername;
    message.content = content;

    // 使用服务器提供的时间戳
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    if (dt.isValid()) {
        message.timestamp = dt;
    } else {
        message.timestamp = QDateTime::currentDateTime();
    }

    message.isSelf = false;

    if (!ChatDatabase::instance()->saveMessage(message)) {
        qWarning() << "保存离线私聊消息失败";
    } else {
        qDebug() << "离线私聊消息已保存:" << content;
    }

    // TODO: 如果需要，可以在这里更新私聊列表显示未读消息数量
}

// 请求好友列表
void MainWindow::requestFriendList() {
    qDebug() << "正在请求好友列表...";

    QJsonObject request;
    request["type"] = "getFriendList";
    request["account"] = currentAccount;

    NetworkManager::instance()->sendMessage(request);
}

// 处理好友列表响应
void MainWindow::onFriendListReceived(const QJsonObject& response) {
    qDebug() << "收到好友列表响应:" << response;

    if (response["status"].toString() == "success") {
        QJsonArray friends = response["friends"].toArray();
        updateFriendListUI(friends);
        qDebug() << "好友列表更新成功，好友数量:" << friends.size();
    } else {
        QString error = response["message"].toString();
        qDebug() << "获取好友列表失败:" << error;
    }
}

// 更新好友列表UI
void MainWindow::updateFriendListUI(const QJsonArray& friends) {
    qDebug() << "开始更新好友列表UI，好友数量:" << friends.size();

    // 先清除现有的好友列表项（保留公共聊天室）
    for (int i = userListWidget->count() - 1; i >= 1; i--) {
        QListWidgetItem* item = userListWidget->item(i);
        QString itemData = item->data(Qt::UserRole).toString();
        if (itemData != "PUBLIC") {
            delete userListWidget->takeItem(i);
        }
    }

    // 如果有好友，添加分隔线
    if (!friends.isEmpty()) {
        QListWidgetItem* separator = new QListWidgetItem("――― 好友列表 ―――");
        separator->setFlags(Qt::NoItemFlags); // 不可选择
        separator->setTextAlignment(Qt::AlignCenter);
        QFont separatorFont;
        separatorFont.setPointSize(9);
        separatorFont.setBold(true);
        separator->setFont(separatorFont);
        separator->setForeground(QColor("#666666"));
        separator->setData(Qt::UserRole, "SEPARATOR");
        userListWidget->addItem(separator);
    }

    // 按在线状态分组显示好友
    QJsonArray onlineFriends;
    QJsonArray offlineFriends;

    for (const auto& friendValue : friends) {
        QJsonObject friendObj = friendValue.toObject();
        bool isOnline = friendObj["isOnline"].toBool();

        if (isOnline) {
            onlineFriends.append(friendValue);
        } else {
            offlineFriends.append(friendValue);
        }
    }

    // 先添加在线好友
    for (const auto& friendValue : onlineFriends) {
        addFriendToList(friendValue.toObject(), true);
    }

    // 再添加离线好友
    for (const auto& friendValue : offlineFriends) {
        addFriendToList(friendValue.toObject(), false);
    }

    qDebug() << "好友列表UI更新完成，在线:" << onlineFriends.size() << "，离线:" << offlineFriends.size();
}

// 添加好友到列表的辅助方法
void MainWindow::addFriendToList(const QJsonObject& friendObj, bool isOnline) {
    QString friendAccount = friendObj["account"].toString();
    QString friendUsername = friendObj["username"].toString();

    // 创建好友列表项
    QString statusIcon = isOnline ? "🟢" : "⚫";
    QString displayText = QString("%1 %2").arg(statusIcon, friendUsername);

    QListWidgetItem* friendItem = new QListWidgetItem(displayText);
    friendItem->setData(Qt::UserRole, friendAccount);      // 存储好友账号
    friendItem->setData(Qt::UserRole + 1, friendUsername); // 存储好友用户名
    friendItem->setData(Qt::UserRole + 2, isOnline);       // 存储在线状态

    // 设置字体和颜色
    QFont friendFont;
    friendFont.setPointSize(10);

    if (isOnline) {
        friendFont.setBold(true);
        friendItem->setForeground(QColor("#2E7D32")); // 深绿色
    } else {
        friendFont.setBold(false);
        friendItem->setForeground(QColor("#757575")); // 灰色
    }

    friendItem->setFont(friendFont);
    userListWidget->addItem(friendItem);
}