//
// Created by 20852 on 25-7-21.
//
#include "login_in.h"
#include "mainwindow.h"
#include "registerwindow.h"
#include <qboxlayout>   // 布局管理器
#include <QMessageBox>   // 用于弹出消息框
#include <QDebug>        // 用于调试输出
#include <QHostAddress> // 用于 QTcpSocket::connectToHost
#include <QJsonDocument> // 用于 JSON 处理
#include <QJsonObject>   // 用于 JSON 处理

// LoginWindow 类的构造函数实现
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent),
      mainTcpSocket(new QTcpSocket(this)), // 初始化 QTcpSocket
      m_blockSize(0)                     // **初始化 m_blockSize**
{
    setWindowTitle(" "); // 设置窗口标题
    setFixedSize(290, 400); //禁用缩放

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint); // 隐藏图标和最大化/最小化按钮，保留关闭按钮

    // --- 1. 创建并设置主布局 ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addStretch();

    // --- 2. 创建账号输入行 ---
    QHBoxLayout *accountLayout = new QHBoxLayout();
    accountLabel = new QLabel("账号:", this);
    accountLineEdit = new QLineEdit(this);
    accountLineEdit->setPlaceholderText("请输入账号");
    accountLineEdit->setStyleSheet("QLineEdit { "
                                   "border-radius: 10px; "
                                   "padding: 5px; border: "
                                   "1px solid gray; "
                                   "}");
    accountLayout->addWidget(accountLabel);
    accountLayout->addWidget(accountLineEdit);
    mainLayout->addLayout(accountLayout);

    mainLayout->addSpacing(15);

    // --- 3. 创建密码输入行 ---
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLabel = new QLabel("密码:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setPlaceholderText("请输入密码");
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setStyleSheet("QLineEdit { "
                                    "border-radius: 10px; "
                                    "padding: 5px; "
                                    "border: 1px solid gray; "
                                    "}");
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordLineEdit);
    mainLayout->addLayout(passwordLayout);

    mainLayout->addSpacing(15);

    // --- 4. 创建登录和注册按钮 ---
    loginButton = new QPushButton("登录", this); // 按钮文本改为“登录/连接”
    loginButton->setStyleSheet("QPushButton {"
                              "    border-radius: 10px;"
                              "    background-color: #1E90FF;"
                              "    color: white;"
                              "    padding: 8px 20px;"
                              "    border: none;"
                              "}"
                              "QPushButton:hover {"
                              "    background-color: #4169E1;"
                              "}"
                              "QPushButton:pressed {"
                              "    background-color: #0854AC;"
                              "}");
    mainLayout->addWidget(loginButton);

    registerButton = new QPushButton("注册", this); // 按钮文本改为“登录/连接”
    registerButton->setStyleSheet("QPushButton {"
                              "    border-radius: 10px;"
                              "    background-color: #1E90FF;"
                              "    color: white;"
                              "    padding: 8px 20px;"
                              "    border: none;"
                              "}"
                              "QPushButton:hover {"
                              "    background-color: #4169E1;"
                              "}"
                              "QPushButton:pressed {"
                              "    background-color: #0854AC;"
                              "}");
    mainLayout->addWidget(registerButton);

    mainLayout->addStretch(); // 使布局更紧凑

    // --- 5. 连接信号与槽 ---
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterButtonClicked);

    // 连接 QTcpSocket 的信号到相应的槽函数
    connect(mainTcpSocket, &QTcpSocket::connected, this, &LoginWindow::onConnected);
    connect(mainTcpSocket, &QTcpSocket::disconnected, this, &LoginWindow::onDisconnected);
    connect(mainTcpSocket, &QTcpSocket::readyRead, this, &LoginWindow::onSocketReadyRead); // 处理服务器响应的关键
    connect(mainTcpSocket, &QTcpSocket::errorOccurred, this, &LoginWindow::onSocketErrorOccurred);

    // **新增：在构造函数末尾调用，客户端启动后自动连接服务器**
    connectToServer();

    qDebug() << "LoginWindow initialized.";
}

LoginWindow::~LoginWindow() {
    // 确保在析构时断开连接并清理 QTcpSocket
    if (mainTcpSocket->state() == QAbstractSocket::ConnectedState) {
        mainTcpSocket->disconnectFromHost();
    }
    // m_tcpSocket 是 QWidget 的子对象，通常会在 QWidget 析构时自动释放
    // 或者如果你在构造函数中没有指定父对象，则需要 delete m_tcpSocket;
    // 这里我们指定了 this 作为父对象，所以不需要显式 delete m_tcpSocket;
    qDebug() << "LoginWindow destroyed.";
}

// ButtonClicked 槽函数的实现
//-点击登录按钮后进行什么操作
void LoginWindow::onLoginButtonClicked() {
    // 确保客户端已经连接到服务器
    if (mainTcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "连接错误", "请先确保已连接到服务器！");
        // 可以在这里再次尝试连接，或者提示用户点击连接按钮（如果你的UI有独立的连接按钮的话）
        // 目前，由于构造函数已经尝试连接，这里只做提示
        return;
    }

    // 从输入框获取文本内容 (账号密码仍然需要)
    QString account = accountLineEdit->text();
    QString password = passwordLineEdit->text();

    // **硬编码目标 IP 和端口**
    const QString TARGET_IP = "127.0.0.1";
    const quint16 TARGET_PORT = 12345;

    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空！");
        return;
    }

    // 禁用按钮和输入框，防止重复发送或修改
    loginButton->setEnabled(false);
    registerButton->setEnabled(false); // 登录或注册请求发送后，通常会禁用两个按钮
    accountLineEdit->setEnabled(false);
    passwordLineEdit->setEnabled(false);

    // 构建登录请求的 JSON 数据
    QJsonObject request;
    request["type"] = "login"; // 请求类型：登录
    request["account"] = account;
    request["password"] = password;

    QByteArray jsonData = QJsonDocument(request).toJson(QJsonDocument::Compact);

    // 准备数据包：长度前缀 + JSON 数据
    QByteArray dataBlock;
    QDataStream out(&dataBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << (quint32)0; // 预留总长度位置
    out << jsonData;   // 写入实际 JSON 数据

    out.device()->seek(0);
    out << (quint32)(dataBlock.size() - sizeof(quint32)); // 回头写入总长度

    // 发送数据
    mainTcpSocket->write(dataBlock);
    mainTcpSocket->flush(); // 强制发送缓冲数据

    qInfo() << "已发送登录请求：用户 " << account;
}

//-点击注册按钮后进行什么操作
void LoginWindow::onRegisterButtonClicked() {
    // 确保 socket 已连接才能打开注册窗口并尝试注册
    if (mainTcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器，无法打开注册界面！");
        return;
    }

    this->hide(); // 隐藏登录窗口
    // **关键修改：将 LoginWindow 持有的 m_tcpSocket 传递给 RegisterWindow 的构造函数**
    RegisterWindow *registerWindow = new RegisterWindow(mainTcpSocket);
    // 注意：这里的 registerWindow 没有指定父对象，因此它的生命周期需要手动管理
    // 或者，如果你想让它在关闭时自动删除，可以使用 registerWindow->setAttribute(Qt::WA_DeleteOnClose);
    // 但目前我们仅用 show()，当它关闭时，LoginWindow 会重新 show()，所以不设置父对象，让其独立。

    // 连接信号与槽，以便在注册窗口关闭时重新显示登录窗口
    connect(registerWindow, &RegisterWindow::registerWindowClosed, this, &LoginWindow::handleRegisterWindowClosed);
    connect(this, &LoginWindow::registerWindowClosed, registerWindow, &RegisterWindow::close);

    registerWindow->show(); // 显示注册窗口

    qDebug() << "注册窗口已打开，并传递了共享 socket。";
}

void LoginWindow::handleRegisterWindowClosed()
{
    qDebug() << "注册窗口已关闭，重新显示登录窗口。"; // 添加一个调试输出，方便确认
    this->show(); // 重新显示登录窗口
}

// QTcpSocket 相关的槽函数实现
//-连接成功
void LoginWindow::onConnected() {
    qInfo() << "成功连接到服务器！";
    // 可以在这里更新 UI 状态，例如提示用户已连接
    // 但不要在这里就显示主窗口，主窗口应该在登录成功响应后才显示
    loginButton->setText("登录"); // 连接成功后，登录按钮应该显示“登录”，并可用
    loginButton->setEnabled(true);
    // accountLineEdit 和 passwordLineEdit 保持可用，等待用户输入登录信息
    accountLineEdit->setEnabled(true);
    passwordLineEdit->setEnabled(true);
    // 注册按钮也应该可用
    registerButton->setEnabled(true);
}

//-处理断开连接情况
void LoginWindow::onDisconnected() {
    loginButton->setText("登录"); // 按钮文本变回“登录”
    loginButton->setEnabled(true);   // 重新启用按钮
    accountLineEdit->setEnabled(true); // 重新启用账号密码输入
    passwordLineEdit->setEnabled(true);
    qDebug() << "已与服务器断开连接。";
}

//-处理服务器响应数据
void LoginWindow::onSocketReadyRead() {
    QDataStream in(mainTcpSocket);
    in.setVersion(QDataStream::Qt_6_0);

    for (;;) {
        if (m_blockSize == 0) {
            if (mainTcpSocket->bytesAvailable() < (qint64)sizeof(quint32)) {
                return; // 数据不足以读取完整的包大小，等待更多数据
            }
            in >> m_blockSize; // 读取数据包的总大小
            qDebug() << "DEBUG (Client): 读取到数据包的预期总长度 (m_blockSize):" << m_blockSize;
        }

        if (mainTcpSocket->bytesAvailable() < m_blockSize) {
            return; // 数据不足以读取完整的 JSON 数据，等待更多数据
        }

        QByteArray jsonData;
        in >> jsonData; // 读取实际的 JSON 数据

        qDebug() << "DEBUG (Client): QDataStream 成功读取到 JSON 数据块。大小:" << jsonData.size();
        qDebug() << "DEBUG (Client): 原始 JSON 数据 (Hex):" << jsonData.toHex();
        qDebug() << "DEBUG (Client): 原始 JSON 数据 (UTF8):" << QString::fromUtf8(jsonData);


        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "客户端无法解析传入的 JSON 数据。原始数据 (Hex):" << jsonData.toHex();
            m_blockSize = 0; // 重置 m_blockSize，准备接收下一个包
            break; // 跳出循环，等待下一个包
        }

        QJsonObject response = doc.object();
        qInfo() << "客户端收到服务器响应：" << response;

        // 调用辅助函数处理解析后的响应
        processResponse(response);

        m_blockSize = 0; // 处理完一个完整的数据包后，重置 m_blockSize，准备接收下一个包
    }
}

//-处理服务器无法连接问题
void LoginWindow::onSocketErrorOccurred(QTcpSocket::SocketError socketError) {
    Q_UNUSED(socketError); // 避免编译器警告，表示这个参数可能不直接使用
    loginButton->setText("登录"); // 按钮文本变回“登录”
    loginButton->setEnabled(true);   // 重新启用按钮
    accountLineEdit->setEnabled(true); // 重新启用账号密码输入
    passwordLineEdit->setEnabled(true);
    qDebug() << "Socket 错误: " << mainTcpSocket->errorString();
    QMessageBox::critical(this, "连接错误", "无法连接到服务器:\n" + mainTcpSocket->errorString());
}

// **connectToServer() 辅助函数
void LoginWindow::connectToServer() {
    if (mainTcpSocket->state() == QAbstractSocket::UnconnectedState) {
        qInfo() << "尝试连接到服务器...";
        // 服务器的 IP 地址和端口号，需要与服务器端保持一致
        mainTcpSocket->connectToHost(QHostAddress::LocalHost, 12345); // 确认这里与服务器一致
    } else {
        qInfo() << "已连接或正在连接到服务器，无需重复连接。当前状态：" << mainTcpSocket->state();
    }
}

// 实现 processResponse() 辅助函数：处理服务器响应**
void LoginWindow::processResponse(const QJsonObject& response) {
    QString responseType = response["type"].toString();
    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (responseType == "register") {
        if (status == "success") {
            int ret = QMessageBox::information(nullptr, "注册成功", message);
            if (ret == QMessageBox::Ok) {
                emit registerWindowClosed(); // 直接发出信号
            }
        } else {
            QMessageBox::warning(this, "注册失败", message);
        }
    }
    // TODO: 将来这里可以添加对 "login" 响应的处理逻辑
    else if (responseType == "login") {
        if (status == "success") {
            QMessageBox::information(this, "登录成功", message);
            this->hide();
            qDebug() << "RegisterWindow: 注册成功，窗口即将关闭。"; // 隐藏登录窗口
            MainWindow *mainWindow = new MainWindow(mainTcpSocket); // **重要：将来把 m_tcpSocket 传递给 MainWindow**
            mainWindow->show(); // 显示主聊天窗口
        } else {
            QMessageBox::warning(this, "登录失败", message);
            // 登录失败后，确保登录按钮可用，并且账号密码输入框可用
            loginButton->setEnabled(true);
            registerButton->setEnabled(true);
            accountLineEdit->setEnabled(true);
            passwordLineEdit->setEnabled(true);
        }
    }
    // else if (responseType == "chatMessage") {
    //
    // }
    else {
        qWarning() << "收到未知响应类型：" << responseType;
        QMessageBox::warning(this, "服务器响应", "收到未知响应。");
    }
}



