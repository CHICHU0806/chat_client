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
#include <QGraphicsDropShadowEffect>  // 添加阴影效果头文件
#include <QMouseEvent>  // 添加鼠标事件头文件
#include <QTextCharFormat>  // 添加文本格式头文件
#include <QFileDialog> // 文件对话框

// 构造函数
MainWindow::MainWindow(QTcpSocket *socket,const QString& username,const QString& account, QWidget *parent)
    : QWidget(parent),
      mainTcpSocket(socket), // **将传入的共享 socket 赋值给成员变量**
      currentUsername(username),
      currentAccount(account),     // 账号ID
      personalMsgWindow(nullptr), // 初始化为空指针
      addFriendWindow(nullptr),
      friendListWindow(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(1000, 750);        // 设置窗口最小大小

    // 创建主容器widget，用于应用阴影效果
    QWidget *mainContainer = new QWidget(this);
    mainContainer->setStyleSheet(
        "background-color: #6690A0;"
        "border-radius: 18px;"
    );

    // 创建阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);           // 阴影模糊半径
    shadowEffect->setXOffset(0);               // X轴偏移
    shadowEffect->setYOffset(0);               // Y轴偏移
    shadowEffect->setColor(QColor(0, 0, 0, 100)); // 阴影颜色和透明度
    mainContainer->setGraphicsEffect(shadowEffect);

    // 保存阴影效果和主容器的引用，用于最大化时的处理
    this->shadowEffect = shadowEffect;
    this->mainContainer = mainContainer;

    // 创建主容器的布局
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(20, 20, 20, 20); // 为阴影留出空间
    containerLayout->addWidget(mainContainer);

    // 保存容器布局的引用
    this->containerLayout = containerLayout;

    QWidget *titleBar = new QWidget(mainContainer);
    titleBar->setFixedHeight(36);
    // titleBar 只设置上方圆角
    titleBar->setStyleSheet(
        "background-color: #3A5A6C;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        "border-bottom-left-radius: 0px;"
        "border-bottom-right-radius: 0px;"
    );

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    QLabel *titleLabel = new QLabel("聊天室", titleBar);
    titleLabel->setStyleSheet("color: white; font-size: 15px;");
    titleLayout->addWidget(titleLabel);

    titleLayout->addStretch();


    QPushButton *minBtn = new QPushButton("-", titleBar);
    QPushButton *maxBtn = new QPushButton("□", titleBar);
    QPushButton *closeBtn = new QPushButton("×", titleBar);

    QString btnStyle =
    "QPushButton {"
    "  background: transparent;"
    "  border: none;"
    "  color: white;"
    "  font-size: 18px;"
    "  min-width: 32px;"
    "  min-height: 32px;"
    "  border-radius: 16px;"
    "  outline: none;"
    "}"
    "QPushButton:focus {"
    "  outline: none;"
    "  border: none;"
    "}"
    "QPushButton:hover {"
    "  background: #607D8B;"
    "}"
    "QPushButton:pressed {"
    "  background: #455A64;"
    "}";

    minBtn->setStyleSheet(btnStyle);
    maxBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(btnStyle.replace("#607D8B", "#E57373").replace("#455A64", "#D32F2F"));

    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(maxBtn);
    titleLayout->addWidget(closeBtn);

    // 创建主布局 - 应用到mainContainer而不是this
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 创建顶部状态栏
    QWidget *topBar = new QWidget(mainContainer);
    topBar->setFixedHeight(50);  // 固定高度
    // topBar 不设置圆角，保持贴合
    topBar->setStyleSheet(
        "background-color: #6690A0;"
        "border-top-left-radius: 0px;"
        "border-top-right-radius: 0px;"
        "border-bottom-left-radius: 0px;"
        "border-bottom-right-radius: 0px;"
        "border-bottom: 1px solid #1E1E1E;"
    );

    // 顶部状态栏的布局
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(10, 0, 10, 0);

    // 创建个人消息按钮（圆形）
    personalMsgButton = new QPushButton(mainContainer);
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
    QPushButton *settingsButton = new QPushButton("⚙", mainContainer);
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
    addFriendButton = new QPushButton("➕", mainContainer);
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
    friendListButton = new QPushButton("👥", mainContainer);
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
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, mainContainer);
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
    QWidget *leftWidget = new QWidget(mainContainer);
    leftWidget->setMinimumWidth(180);  // 添加这行：设置最小宽度
    leftWidget->setMaximumWidth(350);  // 添加这行：设置最大宽度
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // leftWidget 只设置左下圆角，其余为0
    leftWidget->setStyleSheet(
        "background-color: #6690A0;"
        "border-top-left-radius: 0px;"
        "border-top-right-radius: 0px;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 0px;"
    );

    userListWidget = new QListWidget(leftWidget);
    userListWidget->setStyleSheet(
        "QListWidget {"
        "    border: none;"
        "    background-color: #6690A0;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    height: 45px;"
        "    padding: 8px 12px;"
        "    margin: 2px 8px;"
        "    border-radius: 0px;" // 取消圆角
        "    background-color: #6690A0;"
        "    border: 1px solid #e0e0e0;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #607D8B;"
        "    border-color: white;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #009688;"
        "    color: white;"
        "    border-color: #1976d2;"
        "}"
        "QListWidget::item:selected:hover {"
        "    background-color: #009688;"
        "}"
    );

    leftLayout->addWidget(userListWidget);

    // 2. 中间聊天区域
    QWidget *centerWidget = new QWidget(mainContainer);
    centerWidget->setMinimumWidth(400);  // 添加这行：设置聊天区域最小宽度
    // centerWidget 只设置下方圆角，保证底部圆角
    centerWidget->setStyleSheet(
        "background-color: #6690A0;"
        "border-top-left-radius: 0px;"
        "border-top-right-radius: 0px;"
        "border-bottom-left-radius: 18px;"
        "border-bottom-right-radius: 18px;"
    );

    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);

    chatDisplay = new QTextEdit(centerWidget);
    chatDisplay->setReadOnly(true);
    chatDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // 安装事件过滤器，实现QQ式的光标效果
    chatDisplay->installEventFilter(this);
    // 设置默认光标为箭头
    chatDisplay->viewport()->setCursor(Qt::ArrowCursor);
    chatDisplay->setStyleSheet(
        "QTextEdit {"
        "    border: none;"
        "    background-color: #6690A0;"
        "    padding: 10px;"
        "}"
        "QScrollBar:vertical {"
        "    background: #e0e0e0;"
        "    width: 12px;"
        "    margin: 0px 0px 0px 0px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #607D8B;"
        "    min-height: 30px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #455A64;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    background: none;"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
        "    background: none;"
        "    border: none;"
        "}"
    );

    setupScrollBarConnection();

    // 初始化聊天记录相关变量
    currentChatType = "public";
    currentChatTarget = "PUBLIC";
    loadedMessageCount = 0;
    isLoadingHistory = false;

    // 创建输入区域上方的工具栏
    QWidget *toolBar = new QWidget(centerWidget);
    toolBar->setFixedHeight(40);
    toolBar->setStyleSheet(
        "QWidget {"
        "    background-color: #6690A0;"
        "    border: none;"
        "}"
    );

    QHBoxLayout *toolLayout = new QHBoxLayout(toolBar);
    toolLayout->setContentsMargins(10, 5, 10, 5);
    toolLayout->setSpacing(10);

    // 发送文件按钮
    QPushButton *sendFileButton = new QPushButton("📎", toolBar);
    sendFileButton->setFixedSize(30, 30);
    sendFileButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #607D8B;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 15px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #455A64;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #37474F;"
        "}"
    );
    sendFileButton->setToolTip("发送文件");

    // 发送图片按钮
    QPushButton *sendImageButton = new QPushButton("🖼️", toolBar);
    sendImageButton->setFixedSize(30, 30);
    sendImageButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #607D8B;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 15px;"
        "    font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #455A64;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #37474F;"
        "}"
    );
    sendImageButton->setToolTip("发送图片");

    toolLayout->addWidget(sendFileButton);
    toolLayout->addWidget(sendImageButton);
    toolLayout->addStretch(); // 左对齐

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

    mainLayout->insertWidget(0, titleBar);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(mainSplitter);

    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);

    centerLayout->addWidget(chatDisplay);
    centerLayout->addWidget(toolBar);    // 添加工具栏
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

    // 连接重写标题栏按钮信号槽
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maxBtn, &QPushButton::clicked, this, [this]() {
        static bool isProcessing = false;
        if (isProcessing) return;

        isProcessing = true;

        if (isMaximized()) {
            showNormal();
            QTimer::singleShot(50, this, [this]() {
                restoreNormalWindowStyle();
            });
        } else {
            showMaximized();
            QTimer::singleShot(50, this, [this]() {
                setMaximizedWindowStyle();
            });
        }

        QTimer::singleShot(200, []() {
            isProcessing = false;
        });
    });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

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
    connect(NetworkManager::instance(), &NetworkManager::aiAnswerReceived, this, &MainWindow::onAiAnswerReceived);
    connect(network, &NetworkManager::unknownMessageReceived, this, &MainWindow::handleUnknownMessage);

    //连接文件传输相关信号
    connect(network, &NetworkManager::fileTransferResponse, this, &MainWindow::handleFileTransferResponse);
    connect(network, &NetworkManager::fileChunkReceived, this, &MainWindow::handleFileChunkReceived);
    connect(sendFileButton, &QPushButton::clicked, this, &MainWindow::onSendFileButtonClicked);
    connect(sendImageButton, &QPushButton::clicked, this, &MainWindow::onSendImageButtonClicked);

    QTimer::singleShot(500, this, [this]() {
        requestFriendList();
    });

    QTimer::singleShot(500, this, [this]() {
        requestOfflineMessages();
    });

    initializeDownloadDir();
    isOfflineMessagesProcessed = false;

    qDebug() << "MainWindow initialized with shared socket.";
}

// 析构函数
MainWindow::~MainWindow() {
    qDebug() << "MainWindow destroyed.";
    // m_tcpSocket 不需要在这里 delete，因为它是由 LoginWindow 管理的共享指针
}

// 在 MainWindow 类中添加事件处理
void MainWindow::mousePressEvent(QMouseEvent *event) {
    // 只处理拖动（左键按下且在标题栏区域）
    if (event->button() == Qt::LeftButton && event->pos().y() <= 36) {
        if (!isMaximized()) {
            dragging = true;
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        } else {
            // 最大化时的拖拽处理 - 简化逻辑
            dragging = true;

            // 记录鼠标在窗口标题栏中的相对位置
            QPoint mouseInWindow = event->pos();
            QPoint globalMousePos = event->globalPosition().toPoint();

            // 计算鼠标在标题栏中的比例位置
            double mouseRatio = static_cast<double>(mouseInWindow.x()) / width();

            // 还原窗口 - 使用简单的还原方法
            showNormal();

            // 一次性恢复样式，避免重复设置
            restoreNormalWindowStyle();

            // 计算新位置
            QSize normalSize = size(); // 使用当前大小
            int newX = globalMousePos.x() - static_cast<int>(normalSize.width() * mouseRatio);
            int newY = globalMousePos.y() - mouseInWindow.y();

            // 边界检查
            QScreen* screen = QGuiApplication::primaryScreen();
            QRect screenGeometry = screen->availableGeometry();
            newX = qMax(screenGeometry.left(), qMin(newX, screenGeometry.right() - normalSize.width()));
            newY = qMax(screenGeometry.top(), qMin(newY, screenGeometry.bottom() - normalSize.height()));

            // 移动窗口
            move(newX, newY);

            // 重新计算拖动偏移
            dragPosition = globalMousePos - QPoint(newX, newY);
            event->accept();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        // 简化移动逻辑，避免在移动过程中重复设置样式
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (dragging) {
        dragging = false;

        // 只在拖拽到顶部时才最大化
        if (event->globalPosition().toPoint().y() <= 10) {
            showMaximized();
            // 设置最大化样式
            setMaximizedWindowStyle();
        }
        event->accept();
    }
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

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->pos().y() <= 36) {
        // 防止重复触发
        static bool isProcessing = false;
        if (isProcessing) return;

        isProcessing = true;

        if (isMaximized()) {
            showNormal();
            QTimer::singleShot(50, this, [this]() {
                restoreNormalWindowStyle();
            });
        } else {
            showMaximized();
            QTimer::singleShot(50, this, [this]() {
                setMaximizedWindowStyle();
            });
        }

        QTimer::singleShot(200, []() {
            isProcessing = false;
        });

        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
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
        // 传递当前好友列表数据
        friendListWindow->setFriendList(currentFriendListJsonArray);
        connect(friendListWindow, &FriendListWindow::friendSelected,
                this, [this](const QString& friendAccount, const QString& friendUsername) {
                    // 切换到对应好友私聊
                    currentChatType = "private";
                    currentChatTarget = friendAccount;
                    chatDisplay->clear();
                    loadChatHistory(currentChatType, currentChatTarget);
                    setWindowTitle(QString("聊天室 - %1 - 与 %2 的私聊").arg(currentUsername, friendUsername));
                    // 选中左侧用户列表对应好友
                    for (int i = 0; i < userListWidget->count(); ++i) {
                        QListWidgetItem* item = userListWidget->item(i);
                        if (item->data(Qt::UserRole).toString() == friendAccount) {
                            userListWidget->setCurrentItem(item);
                            break;
                        }
                    }
                });
        connect(friendListWindow, &FriendListWindow::requestFriendList, this, &MainWindow::requestFriendList);
    } else {
        // 每次打开都刷新好友列表
        friendListWindow->setFriendList(currentFriendListJsonArray);
    }

    friendListWindow->show();
    friendListWindow->raise();
    friendListWindow->activateWindow();
}

//离线消息处理
void MainWindow::handleOfflineMessages(const QJsonObject& response) {
    qDebug() << "=== 开始处理离线消息响应 ===";
    qDebug() << "收到离线消息响应:" << response;

    QJsonArray messages = response["messages"].toArray();
    int msgCount = messages.size();
    if (msgCount > 0) {
        qDebug() << "收到推送的离线消息数量:" << msgCount;
    } else {
        qDebug() << "未收到任何离线消息推送（messages 数组为空）";
    }

    // 关键改动：将离线消息直接保存到数据库
    for (const QJsonValue &messageValue : messages) {
        QJsonObject message = messageValue.toObject();
        QString type = message["type"].toString();
        QString status = message["status"].toString();
        QString sender = message["sender"].toString(); // 账号
        QString username = message["username"].toString(); // 用户名
        QString content = message["content"].toString();
        QString timestamp = message["timestamp"].toString();

        if (type == "chatMessage" && (status == "offline_broadcast" || status == "offline_private")) {
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
            ChatDatabase::instance()->saveMessage(msg);
        }
    }

    // 标记离线消息已处理
    isOfflineMessagesProcessed = true;

    // 关键改动：根据当前聊天类型和目标重新加载历史记录
    qDebug() << "离线消息处理完成，根据当前聊天窗口加载历史记录。";
    if (currentChatType == "public") {
        loadChatHistory("public", "PUBLIC");
    } else if (currentChatType == "private" && !currentChatTarget.isEmpty()) {
        loadChatHistory("private", currentChatTarget);
    }

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
        "    border-radius: 0px;"             // 取消圆角
        "    background-color: #6690A0;"        // 默认背景色
        "    border: 1px solid #e0e0e0;"      // 边框
        "}"
        "QListWidget::item:hover {"
        "    background-color: #607D8B;"      // hover时的背景色（浅蓝色）
        "    border-color: white;"          // hover时的边框色
        "}"
        "QListWidget::item:selected {"
        "    background-color: #009688;"      // 选中时的背景色（蓝色）
        "    color: white;"
        "    border-color: #1976d2;"
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

    // 添加AI问答入口
    aiListItem = new QListWidgetItem("🤖 AI问答");
    aiListItem->setData(Qt::UserRole, "AI");
    QFont aiFont;
    aiFont.setPointSize(11);
    aiFont.setBold(true);
    aiListItem->setFont(aiFont);
    userListWidget->addItem(aiListItem);
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
    else if (chatTarget == "AI") {
        // 切换到AI问答界面
        currentChatType = "ai";
        currentChatTarget = "AI";
        chatDisplay->clear(); // 清空聊天区域
        loadChatHistory("ai", "AI"); // 加载AI历史
        setWindowTitle(QString("聊天室 - %1 - AI问答").arg(currentUsername));
        // AI 问答内容会通过 onAiAnswerReceived 显示在 chatDisplay
        return;
    }
    else if (chatTarget == "SEPARATOR") {
        // 分隔线，不做任何操作
        return;
    }
    else {
        // 切换到好友私聊
        QString friendUsername = item->data(Qt::UserRole + 1).toString();
        currentChatType = "private";
        currentChatTarget = item->data(Qt::UserRole).toString(); // 好友账号
        chatDisplay->clear();
        loadChatHistory(currentChatType, currentChatTarget);
        setWindowTitle(QString("聊天室 - %1 - 与 %2 的私聊")
                      .arg(currentUsername, friendUsername));
        qDebug() << "切换到好友私聊:" << friendUsername << "(" << currentChatTarget << ")";
        // 清除未读标记
        int unread = item->data(Qt::UserRole + 2).toInt();
        if (unread > 0) {
            item->setData(Qt::UserRole + 2, 0);
            item->setText(friendUsername);
        }
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
        if (username != currentUsername) // 不显示自己
        {
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
void MainWindow::handleChatMessage(const QJsonObject &message) {
    QString type = message["type"].toString();
    QString status = message["status"].toString();
    QString sender = message["sender"].toString(); // 账号
    QString username = message["username"].toString(); // 用户名
    QString content = message["content"].toString();
    QString timestamp = message["timestamp"].toString();

    // 只处理聊天消息
    if (type != "chatMessage") {
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

    // 新增：私聊未读消息提示
    if (status == "private") {
        // 如果当前不是与 sender 的私聊界面，则加未读标记
        if (!(currentChatType == "private" && currentChatTarget == sender)) {
            // 在用户列表找到对应好友项，加未读标记
            for (int i = 0; i < userListWidget->count(); ++i) {
                QListWidgetItem* item = userListWidget->item(i);
                if (item->data(Qt::UserRole).toString() == sender) {
                    // 用 UserRole+2 存未读数
                    int unread = item->data(Qt::UserRole + 2).toInt();
                    unread++;
                    item->setData(Qt::UserRole + 2, unread);
                    // 在文本前加红点或数字
                    QString baseName = item->data(Qt::UserRole + 1).toString();
                    if (unread > 0) {
                        // 用黑色圆点（\u25CF）和数字，设置前景色为红色
                        item->setText(QString("%1  \u25CF%2").arg(baseName).arg(unread)); // \u25CF 是黑色圆点
                        item->setForeground(QBrush(QColor("#D32F2F")));
                    } else {
                        item->setText(baseName);
                        item->setForeground(QBrush(Qt::black)); // 恢复默认颜色
                    }
                    break;
                }
            }
            // 仍然保存到数据库
            saveChatMessage("private", sender, sender, username, content, false);
            return;
        }
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
        handlePublicChatMessage(sender, username, content, displayTime);
   } else if (status == "private") {
        handlePrivateChatMessage(sender, username, content, displayTime);
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
    } else if (currentChatType == "private") {
        message["type"] = "chatMessage";  // 私聊使用不同的类型
        message["chatType"] = "private";
        message["targetAccount"] = currentChatTarget;  // 私聊需要目标账号
    } else if (currentChatType == "ai") {
        message["type"] = "chatMessage";
        message["chatType"] = "ai";
        message["account"] = currentAccount;
        message["question"] = msg;

        saveChatMessage("ai", "AI", currentAccount, currentUsername, msg, true);
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
void MainWindow::handlePublicChatMessage(const QString& senderAccount, const QString& senderUsername, const QString& content, const QString& timestamp) {
    if (currentChatType == "public") {
        QString displayName = (senderAccount == currentAccount) ? "我" : senderUsername;
        // 使用table布局实现左对齐 - 别人的消息
        QString bubbleHtml = QString(
            "<table width='100%' style='margin: 10px 0; border-collapse: collapse;'>"
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
    }
    // 保存到数据库
    saveChatMessage("public", "PUBLIC", senderAccount, senderUsername, content, senderAccount == currentAccount);
}

//处理私聊消息
void MainWindow::handlePrivateChatMessage(const QString& senderAccount, const QString& senderUsername, const QString& content, const QString& timestamp) {
    if (currentChatType == "private" && currentChatTarget == senderAccount) {
        QString displayName = (senderAccount == currentAccount) ? "我" : senderUsername;
        // 使用与公共消息相同的左对齐气泡样式
        QString bubbleHtml = QString(
            "<table width='100%' style='margin: 10px 0; border-collapse: collapse;'>"
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
    }
    saveChatMessage("private", currentChatTarget, senderAccount, senderUsername, content, senderAccount == currentAccount);
}

//聊天记录加载
void MainWindow::loadChatHistory(const QString& chatType, const QString& chatTarget) {
    qDebug() << "=== loadChatHistory 开始 ===";
    qDebug() << "聊天类型:" << chatType << "聊天目标:" << chatTarget;

    currentChatType = chatType;
    currentChatTarget = chatTarget;
    loadedMessageCount = 0;
    isLoadingHistory = false;

    chatDisplay->clear();

    int pageSize = 30; // 每页加载30条
    int totalCount = ChatDatabase::instance()->getMessageCount(chatType, chatTarget);
    QList<ChatMessage> messages = ChatDatabase::instance()->getRecentMessages(chatType, chatTarget, pageSize);

    loadedMessageCount = messages.size();

    qDebug() << "从本地数据库加载了" << messages.size() << "条历史记录";

    if (messages.isEmpty()) {
        qDebug() << "没有历史记录可显示";
        return;
    }

    // 批量拼接HTML，一次性插入
    QString html;
    for (const auto& message : messages) {
        html += formatMessage(message);
    }
    chatDisplay->insertHtml(html);

    // 设置光标到末尾并滚动
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
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

    // 使用数据库的防重复机制
    bool saved = ChatDatabase::instance()->saveMessage(message);
    if (!saved) {
        qWarning() << "保存消息失败:" << content.left(20);
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
        qWarning() << "保存离线消息失败";
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
        currentFriendListJsonArray = friends; // 保存到成员变量
        updateFriendListUI(friends);
        qDebug() << "好友列表更新成功，好友数量:" << friends.size();
    } else {
        QString error = response["message"].toString();
        qDebug() << "获取好友列表失败:" << error;
    }
}

void MainWindow::onAiAnswerReceived(const QJsonObject& resp) {
    QString status = resp["status"].toString();
    QString answer = resp["content"].toString(); // AI回复内容

    if (status == "success") {
        // 显示
        chatDisplay->append(QString("<b>AI：</b>%1").arg(answer));

        // 保存AI回复到数据库
        saveChatMessage("ai", "AI", "AI", "AI助手", answer, false);
    } else {
        QString errorMsg = resp["message"].toString();
        chatDisplay->append(QString("<span style='color:red;'><b>AI服务错误：</b>%1</span>").arg(errorMsg));
    }
}

// 更新好友列表UI
void MainWindow::updateFriendListUI(const QJsonArray& friends) {
    qDebug() << "开始更新好友列表UI，好友数量:" << friends.size();

    // 先清除现有的好友列表项
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
void MainWindow::addFriendToList(const QJsonObject& friendObj, bool /*isOnline*/) {
    QString friendAccount = friendObj["account"].toString();
    QString friendUsername = friendObj["username"].toString();

    // 只显示用户名
    QString displayText = friendUsername;

    QListWidgetItem* friendItem = new QListWidgetItem(displayText);
    friendItem->setData(Qt::UserRole, friendAccount);      // 存储好友账号
    friendItem->setData(Qt::UserRole + 1, friendUsername); // 存储好友用户名
    friendItem->setData(Qt::UserRole + 2, 0); // 新增：未读数

    // 设置字体（不区分在线/离线）
    QFont friendFont;
    friendFont.setPointSize(10);
    friendFont.setBold(false);
    friendItem->setFont(friendFont);

    userListWidget->addItem(friendItem);
}

void MainWindow::restoreNormalWindowStyle() {
    // 安全地处理阴影效果
    if (!this->mainContainer->graphicsEffect()) {
        // 只有在没有阴影效果时才重新创建
        QGraphicsDropShadowEffect* newShadowEffect = new QGraphicsDropShadowEffect(this);
        newShadowEffect->setBlurRadius(20);
        newShadowEffect->setXOffset(0);
        newShadowEffect->setYOffset(0);
        newShadowEffect->setColor(QColor(0, 0, 0, 100));
        this->mainContainer->setGraphicsEffect(newShadowEffect);
        this->shadowEffect = newShadowEffect; // 更新引用
    }

    this->containerLayout->setContentsMargins(20, 20, 20, 20);
    this->mainContainer->setStyleSheet(
        "background-color: #6690A0;"
        "border-radius: 18px;"
    );

    // 强制刷新
    this->mainContainer->update();
}

void MainWindow::setMaximizedWindowStyle() {
    // 安全地移除阴影效果
    if (this->mainContainer->graphicsEffect()) {
        this->mainContainer->setGraphicsEffect(nullptr);
    }

    this->containerLayout->setContentsMargins(0, 0, 0, 0);
    this->mainContainer->setStyleSheet(
        "background-color: #6690A0;"
        "border-radius: 0px;"
    );

    // 强制刷新
    this->mainContainer->update();
}

// 实现事件过滤器，处理聊天区域的鼠标光标
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == chatDisplay) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // 获取鼠标位置对应的文字光标
            QTextCursor cursor = chatDisplay->cursorForPosition(mouseEvent->pos());

            // 检查光标位置是否有文字内容
            cursor.select(QTextCursor::WordUnderCursor);
            QString selectedText = cursor.selectedText();

            // 如果有文字内容，显示文本选择光标；否则显示箭头光标
            if (!selectedText.trimmed().isEmpty()) {
                chatDisplay->viewport()->setCursor(Qt::IBeamCursor);
            } else {
                chatDisplay->viewport()->setCursor(Qt::ArrowCursor);
            }
        }
        else if (event->type() == QEvent::Leave) {
            // 鼠标离开时恢复默认箭头光标
            chatDisplay->viewport()->setCursor(Qt::ArrowCursor);
        }
    }

    return QWidget::eventFilter(obj, event);
}

// 文件传输相关槽函数实现
void MainWindow::handleFileTransferResponse(const QJsonObject& response) {
    processFileTransfer(response);
}

void MainWindow::handleFileChunkReceived(const QByteArray& data) {
    processFileChunk(data);
}

// 文件传输相关槽函数实现
void MainWindow::onSendFileButtonClicked() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "选择要发送的文件",
        QString(),
        "所有文件 (*.*)");

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        qint64 fileSize = fileInfo.size();

        // 检查文件大小限制（例如100MB）
        const qint64 maxFileSize = 100 * 1024 * 1024; // 100MB
        if (fileSize > maxFileSize) {
            QMessageBox::warning(this, "文件过大",
                QString("文件大小不能超过 %1 MB").arg(maxFileSize / 1024 / 1024));
            return;
        }

        // 根据当前聊天类型发送文件
        if (currentChatType == "public" || currentChatType == "private") {
            sendFileTransfer(filePath, currentChatType, currentChatTarget);

            // 在聊天区域显示文件发送信息
            QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
            QString fileMessage = QString("📎 正在发送文件: %1 (%2)")
                .arg(fileInfo.fileName())
                .arg(formatFileSize(fileSize));

            QString bubbleHtml = QString(
                "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
                "<tr><td style='text-align: right; vertical-align: top;'>"
                "<div style='display: inline-block; text-align: right; max-width: 60%;'>"
                "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
                "<div><span style='background-color: #1E90FF; color: white; border-radius: 18px; "
                "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
                "text-align: left; display: inline-block; white-space: pre-wrap; "
                "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
                "</div></td></tr></table>"
            ).arg(currentUsername, timestamp, fileMessage.toHtmlEscaped());

            QTextCursor cursor = chatDisplay->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertHtml(bubbleHtml);
            chatDisplay->setTextCursor(cursor);
            chatDisplay->ensureCursorVisible();
        } else {
            QMessageBox::information(this, "提示", "当前聊天类型不支持文件传输");
        }
    }
}

void MainWindow::onSendImageButtonClicked() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "选择要发送的图片",
        QString(),
        "图片文件 (*.png *.jpg *.jpeg *.gif *.bmp)");

    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        qint64 fileSize = fileInfo.size();

        // 检查图片文件大小限制（例如20MB）
        const qint64 maxImageSize = 20 * 1024 * 1024; // 20MB
        if (fileSize > maxImageSize) {
            QMessageBox::warning(this, "图片过大",
                QString("图片大小不能超过 %1 MB").arg(maxImageSize / 1024 / 1024));
            return;
        }

        // 根据当前聊天类型发送图片
        if (currentChatType == "public" || currentChatType == "private") {
            sendFileTransfer(filePath, currentChatType, currentChatTarget);

            // 在聊天区域显示图片发送信息
            QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
            QString imageMessage = QString("🖼️ 正在发送图片: %1 (%2)")
                .arg(fileInfo.fileName())
                .arg(formatFileSize(fileSize));

            QString bubbleHtml = QString(
                "<table width='100%' style='margin: 15px 0; border-collapse: collapse;'>"
                "<tr><td style='text-align: right; vertical-align: top;'>"
                "<div style='display: inline-block; text-align: right; max-width: 60%;'>"
                "<div style='color: #666; font-size: 10px; margin-bottom: 3px;'>%1 %2</div>"
                "<div><span style='background-color: #1E90FF; color: white; border-radius: 18px; "
                "padding: 10px 16px; word-wrap: break-word; font-size: 14px; line-height: 1.4; "
                "text-align: left; display: inline-block; white-space: pre-wrap; "
                "max-width: 100%; box-sizing: border-box;'>%3</span></div>"
                "</div></td></tr></table>"
            ).arg(currentUsername, timestamp, imageMessage.toHtmlEscaped());

            QTextCursor cursor = chatDisplay->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertHtml(bubbleHtml);
            chatDisplay->setTextCursor(cursor);
            chatDisplay->ensureCursorVisible();
        } else {
            QMessageBox::information(this, "提示", "当前聊天类型不支持图片传输");
        }
    }
}

// 初始化下载目录
void MainWindow::initializeDownloadDir() {
    downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadDir.isEmpty()) {
        downloadDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    QDir dir(downloadDir);
    if (!dir.exists()) {
        dir.mkpath(downloadDir);
    }

    qDebug() << "文件下载目录初始化为:" << downloadDir;
}

// 发送文件传输
void MainWindow::sendFileTransfer(const QString& filePath, const QString& chatType, const QString& chatTarget) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开文件:" << filePath;
        onFileTransferError("", "无法打开文件: " + filePath);
        return;
    }

    QFileInfo fileInfo(filePath);
    QString transferId = QUuid::createUuid().toString();

    // 发送文件元数据
    QJsonObject metadata;
    metadata["type"] = "fileTransfer";
    metadata["action"] = "start";
    metadata["transferId"] = transferId;
    metadata["fileName"] = fileInfo.fileName();
    metadata["fileSize"] = file.size();
    metadata["senderAccount"] = currentAccount;
    metadata["senderUsername"] = currentUsername;
    metadata["chatType"] = chatType;
    metadata["chatTarget"] = chatTarget;

    NetworkManager::instance()->sendMessage(metadata);

    // 分块发送文件
    const int chunkSize = 4096; // 4KB 块大小
    QByteArray buffer;

    while (!file.atEnd()) {
        buffer = file.read(chunkSize);

        // 构造文件数据包
        QByteArray filePacket = "FILE_CHUNK:" + transferId.toUtf8() + ":" + buffer;
        NetworkManager::instance()->sendRawData(filePacket);

        // 可以在这里添加进度更新
        QCoreApplication::processEvents();
    }

    // 发送文件传输完成信号
    QJsonObject completion;
    completion["type"] = "fileTransfer";
    completion["action"] = "complete";
    completion["transferId"] = transferId;

    NetworkManager::instance()->sendMessage(completion);

    file.close();
    qDebug() << "文件发送完成:" << fileInfo.fileName();
}

// 处理文件块数据
void MainWindow::processFileChunk(const QByteArray& data) {
    // 解析文件数据包：FILE_CHUNK:transferId:data 或 FILE_DATA:transferId:data
    QList<QByteArray> parts = data.split(':');
    if (parts.size() < 3) {
        qDebug() << "无效的文件数据包格式";
        return;
    }

    QString transferId = QString::fromUtf8(parts[1]);
    QByteArray fileData = data.mid(parts[0].size() + parts[1].size() + 2); // 跳过前缀transferId

    if (!activeTransfers.contains(transferId)) {
        qDebug() << "未知的传输ID:" << transferId;
        return;
    }

    FileTransferInfo& transfer = activeTransfers[transferId];

    if (transfer.file && transfer.file->write(fileData) == -1) {
        // 发送错误反馈
        sendFileTransferAck(transferId, "error", "写入文件失败");
        onFileTransferError(transferId, "写入文件失败");
        transfer.file->close();
        delete transfer.file;
        activeTransfers.remove(transferId);
        return;
    }

    qint64 previousBytes = transfer.bytesReceived;
    transfer.bytesReceived += fileData.size();

    // 定期发送进度反馈（每接收100KB或传输完成时发送）
    if ((transfer.bytesReceived / 102400) > (previousBytes / 102400) || transfer.bytesReceived >= transfer.fileSize) {
        sendFileTransferProgress(transferId, transfer.bytesReceived, transfer.fileSize);
    }

    onFileTransferProgress(transferId, transfer.bytesReceived, transfer.fileSize);

    // 检查是否传输完成
    if (transfer.bytesReceived >= transfer.fileSize) {
        transfer.file->close();
        QString filePath = transfer.file->fileName();
        delete transfer.file;
        activeTransfers.remove(transferId);

        // 发送完成确认
        sendFileTransferAck(transferId, "completed", "文件传输完成");

        onFileTransferCompleted(transferId, filePath);
        qDebug() << "文件接收完成:" << filePath;
    }
}

// 处理文件传输响应
void MainWindow::processFileTransfer(const QJsonObject& response) {
    QString action = response["action"].toString();

    if (action == "start") {
        QString transferId = response["transferId"].toString();
        QString fileName = response["fileName"].toString();
        qint64 fileSize = response["fileSize"].toInteger();
        QString senderAccount = response["senderAccount"].toString();
        QString senderUsername = response["senderUsername"].toString();
        QString chatType = response["chatType"].toString();
        QString chatTarget = response["chatTarget"].toString();

        // 修正：确保下载目录存在，若不存在则创建
        QDir dir(downloadDir);
        if (!dir.exists()) {
            if (!dir.mkpath(downloadDir)) {
                QString errorDetail = QString("下载目录不存在且创建失败: %1").arg(downloadDir);
                qWarning() << errorDetail;
                sendFileTransferAck(transferId, "error", errorDetail);
                onFileTransferError(transferId, errorDetail);
                return;
            }
        }

        // 创建接收文件
        QString filePath = downloadDir + "/" + fileName;
        QFile* file = new QFile(filePath);

        // 新增：详细日志和错误原因输出
        if (!file->open(QIODevice::WriteOnly)) {
            QString errorDetail = file->errorString();
            qWarning() << "无法创建文件:" << filePath << "错误:" << errorDetail;
            sendFileTransferAck(transferId, "error", "无法创建文件: " + errorDetail);
            onFileTransferError(transferId, "无法创建文件: " + errorDetail);
            delete file;
            return;
        }

        FileTransferInfo transfer;
        transfer.transferId = transferId;
        transfer.fileName = fileName;
        transfer.fileSize = fileSize;
        transfer.bytesReceived = 0;
        transfer.senderAccount = senderAccount;
        transfer.senderUsername = senderUsername;
        transfer.chatType = chatType;
        transfer.chatTarget = chatTarget;
        transfer.file = file;
        transfer.isComplete = false;

        activeTransfers[transferId] = transfer;

        // 发送接收确认
        sendFileTransferAck(transferId, "received", "文件传输已开始");

        onFileTransferStarted(transferId, fileName, fileSize, senderUsername);
    }
    else if (action == "complete") {
        QString transferId = response["transferId"].toString();
        qDebug() << "服务器确认文件传输完成:" << transferId;
    }
}

// 发送文件传输确认
void MainWindow::sendFileTransferAck(const QString& transferId, const QString& status, const QString& message) {
    QJsonObject ack;
    ack["type"] = "fileTransferAck";
    ack["transferId"] = transferId;
    ack["status"] = status; // "received", "completed", "error"
    if (!message.isEmpty()) {
        ack["message"] = message;
    }
    ack["timestamp"] = QDateTime::currentDateTime().toSecsSinceEpoch();

    NetworkManager::instance()->sendMessage(ack);
}

// 发送文件传输进度
void MainWindow::sendFileTransferProgress(const QString& transferId, qint64 bytesReceived, qint64 totalSize) {
    QJsonObject progress;
    progress["type"] = "fileTransferProgress";
    progress["transferId"] = transferId;
    progress["bytesReceived"] = bytesReceived;
    progress["totalSize"] = totalSize;
    progress["percentage"] = static_cast<int>((bytesReceived * 100) / totalSize);
    progress["timestamp"] = QDateTime::currentDateTime().toSecsSinceEpoch();

    NetworkManager::instance()->sendMessage(progress);
}

void MainWindow::onFileTransferError(const QString& transferId, const QString& errorMsg) {
    // 可弹窗或日志，实际可自定义
    qWarning() << "文件传输错误:" << transferId << errorMsg;
    QMessageBox::warning(this, "文件传输错误", errorMsg);
}

void MainWindow::onFileTransferProgress(const QString& transferId, qint64 bytesReceived, qint64 totalSize) {
    qDebug() << "文件传输进度:" << transferId << bytesReceived << "/" << totalSize;
}

void MainWindow::onFileTransferCompleted(const QString& transferId, const QString& filePath) {
    // 文件接收完成后可弹窗或日志
    qDebug() << "文件传输完成:" << transferId << filePath;
    QMessageBox::information(this, "文件接收完成", QString("文件已保存到: %1").arg(filePath));
}

void MainWindow::onFileTransferStarted(const QString& transferId, const QString& fileName, qint64 fileSize, const QString& senderUsername) {
    // 文件传输开始时可弹窗或日志
    qDebug() << "文件传输开始:" << transferId << fileName << fileSize << senderUsername;
}

// static成员实现
QString MainWindow::formatFileSize(qint64 bytes) {
    if (bytes < 1024)
        return QString::number(bytes) + " B";
    else if (bytes < 1024 * 1024)
        return QString::number(bytes / 1024.0, 'f', 2) + " KB";
    else if (bytes < 1024 * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024), 'f', 2) + " MB";
    else
        return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
}


void MainWindow::handleUnknownMessage(const QJsonObject& obj) {
    // 只在调试模式下输出收到的未知类型 JSON 内容到调试控制台
    qDebug() << "[未知消息类型]" << QJsonDocument(obj).toJson(QJsonDocument::Indented);
}
