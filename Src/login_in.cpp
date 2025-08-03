//
// Created by 20852 on 25-7-21.
//
#include "login_in.h"
#include "mainwindow.h"
#include "registerwindow.h"
#include "networkmanager.h"
#include <qboxlayout>   // 布局管理器
#include <QMessageBox>   // 用于弹出消息框
#include <QDebug>        // 用于调试输出
#include <QHostAddress> // 用于 QTcpSocket::connectToHost
#include <QJsonDocument> // 用于 JSON 处理
#include <QJsonObject>   // 用于 JSON 处理

// LoginWindow 类的构造函数实现
LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
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

    // 连接 NetworkManager 的信号
    auto* network = NetworkManager::instance();
    connect(network, &NetworkManager::loginResponse,this, &LoginWindow::handleLoginResponse);
    qDebug() << "LoginWindow initialized.";
}

LoginWindow::~LoginWindow() {
    qDebug() << "LoginWindow destroyed.";
}

// ButtonClicked 槽函数的实现
//-点击登录按钮后进行什么操作
void LoginWindow::onLoginButtonClicked() {
    // 从输入框获取文本内容 (账号密码仍然需要)
    QString account = accountLineEdit->text();
    QString password = passwordLineEdit->text();

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

    //发送登录请求
    NetworkManager::instance()->sendMessage(request);

    qInfo() << "已发送登录请求：用户 " << account;
}

//-点击注册按钮后进行什么操作
void LoginWindow::onRegisterButtonClicked() {
    this->hide(); // 隐藏登录窗口
    // **关键修改：将 LoginWindow 持有的 m_tcpSocket 传递给 RegisterWindow 的构造函数**
    RegisterWindow *registerWindow = new RegisterWindow(NetworkManager::instance()->getSocket());
    // 注意：这里的 registerWindow 没有指定父对象，因此它的生命周期需要手动管理
    // 或者，如果你想让它在关闭时自动删除，可以使用 registerWindow->setAttribute(Qt::WA_DeleteOnClose);
    // 但目前我们仅用 show()，当它关闭时，LoginWindow 会重新 show()，所以不设置父对象，让其独立。

    // 连接信号与槽，以便在注册窗口关闭时重新显示登录窗口
    connect(registerWindow, &RegisterWindow::registerWindowClosed, this, &LoginWindow::handleRegisterWindowClosed);
    registerWindow->show(); // 显示注册窗口

    qDebug() << "注册窗口已打开，并传递了共享 socket。";
}

void LoginWindow::handleRegisterWindowClosed()
{
    qDebug() << "注册窗口已关闭，重新显示登录窗口。"; // 添加一个调试输出，方便确认
    this->show(); // 重新显示登录窗口
}

void LoginWindow::handleLoginResponse(const QJsonObject& response) {
    // 无论成功失败都恢复 UI 状态
    loginButton->setEnabled(true);
    registerButton->setEnabled(true);
    accountLineEdit->setEnabled(true);
    passwordLineEdit->setEnabled(true);

    // 处理登录响应
    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        QString username = response["username"].toString();  // 获取用户名
        this->hide();
        MainWindow *mainWindow = new MainWindow(NetworkManager::instance()->getSocket(), username);  // 传递用户名
        mainWindow->show();
    } else {
        QMessageBox::warning(this, "登录失败", message);
        // 登录失败后恢复按钮和输入框
        loginButton->setEnabled(true);
        registerButton->setEnabled(true);
        accountLineEdit->setEnabled(true);
        passwordLineEdit->setEnabled(true);
    }
}