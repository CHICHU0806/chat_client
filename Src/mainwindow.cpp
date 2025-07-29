//
// Created by 20852 on 25-7-22.
//
#include "mainwindow.h"
#include "sockethandler.h"
#include <qboxlayout> // 用于布局
#include <QHostAddress> // 用于 QTcpSocket
#include <QJsonDocument> // 用于 JSON 处理
#include <QJsonObject>   // 用于 JSON 处理
#include <QDateTime>     // 用于显示消息时间

// 构造函数
MainWindow::MainWindow(QTcpSocket *socket, QWidget *parent)
    : QWidget(parent),
      mainTcpSocket(socket), // **将传入的共享 socket 赋值给成员变量**
      m_blockSize(0)       // 初始化数据包大小为0
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
        "    background-color: #009688;"  // 深色背景
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
        "    background-color: #00796B;"
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

    // 设置初始大小比例
    QList<int> sizes;
    sizes << 200 << 1000;  // 左侧固定200像素，右侧占据剩余空间
    mainSplitter->setSizes(sizes);

    // 将分割器添加到主布局
    mainLayout->addWidget(mainSplitter);

    // 连接信号与槽
    // 连接发送按钮的 clicked 信号到 onSendMessageButtonClicked 槽
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    // 允许按 Enter 键发送消息
    connect(messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendButtonClicked);


    // **连接共享 QTcpSocket 的信号到 MainWindow 的槽函数**
    // 注意：这里的 socket 已经由 LoginWindow 连接成功并传递过来
    connect(mainTcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(mainTcpSocket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(mainTcpSocket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketErrorOccurred);

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

// **槽函数：接收服务器数据**
void MainWindow::onSocketReadyRead() {
    QDataStream in(mainTcpSocket);
    in.setVersion(QDataStream::Qt_6_0); // 确保与服务器端版本一致

    for (;;) {
        if (m_blockSize == 0) {
            if (mainTcpSocket->bytesAvailable() < (qint64)sizeof(quint32)) {
                return; // 数据不足以读取完整的包大小，等待更多数据
            }
            in >> m_blockSize; // 读取数据包的总大小
            qDebug() << "DEBUG (MainWindow): 读取到数据包的预期总长度 (m_blockSize):" << m_blockSize;
        }

        if (mainTcpSocket->bytesAvailable() < m_blockSize) {
            return; // 数据不足以读取完整的 JSON 数据，等待更多数据
        }

        QByteArray jsonData;
        in >> jsonData; // 读取实际的 JSON 数据

        qDebug() << "DEBUG (MainWindow): QDataStream 成功读取到 JSON 数据块。大小:" << jsonData.size();
        qDebug() << "DEBUG (MainWindow): 原始 JSON 数据 (UTF8):" << QString::fromUtf8(jsonData);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "MainWindow 无法解析传入的 JSON 数据。原始数据 (Hex):" << jsonData.toHex();
            m_blockSize = 0; // 重置 m_blockSize，准备接收下一个包
            break; // 跳出循环，等待下一个包
        }

        QJsonObject message = doc.object();
        qInfo() << "MainWindow 收到服务器消息：" << message;

        // 调用辅助函数处理解析后的消息
        processIncomingMessage(message);

        m_blockSize = 0; // 处理完一个完整的数据包后，重置 m_blockSize，准备接收下一个包
    }
}

// **槽函数：处理服务器断开连接**
void MainWindow::onSocketDisconnected() {
    qWarning() << "已从服务器断开连接！聊天功能已禁用。";
    chatDisplay->append("<font color='red'>--- 与服务器断开连接 ---</font>");
    messageInput->setEnabled(false); // 禁用输入框
    sendButton->setEnabled(false);   // 禁用发送按钮
    m_blockSize = 0; // 断开连接后重置数据包大小
}

// **槽函数：处理网络错误**
void MainWindow::onSocketErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError); // 避免编译器警告
    qCritical() << "套接字错误 (MainWindow)：" << mainTcpSocket->errorString();
    QMessageBox::critical(this, "网络错误", "与服务器连接时发生错误：" + mainTcpSocket->errorString());
    onSocketDisconnected(); // 发生错误通常也意味着连接不可用
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

// **辅助函数：处理接收到的 JSON 消息并显示**
void MainWindow::processIncomingMessage(const QJsonObject& message) {
    QString messageType = message["type"].toString();

    if (messageType == "chatMessage") {
        QString sender = message["sender"].toString();
        QString content = message["content"].toString();
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss"); // 获取当前时间作为消息时间

        // 将消息显示到聊天区域
        chatDisplay->append(QString("[%1] <b>%2:</b> %3")
                             .arg(timestamp)
                             .arg(sender)
                             .arg(content));
    } else {
        qWarning() << "MainWindow 收到未知消息类型：" << messageType;
    }
}

// **辅助函数：将消息发送到服务器**
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