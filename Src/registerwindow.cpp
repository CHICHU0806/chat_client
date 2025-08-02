//
// Created by 20852 on 25-7-22.
//
#include "registerwindow.h"
#include "networkmanager.h" // 引入网络管理器头文件
#include <qdialog.h>
#include <qboxlayout>   // 布局管理器
#include <QMessageBox>   // 用于弹出消息框
#include <QDebug>        // 用于调试输出
#include <QJsonDocument> // 用于构建和解析JSON数据
#include <QJsonObject>   // 用于构建JSON对象

// 构造函数实现
RegisterWindow::RegisterWindow(QTcpSocket *socket, QWidget *parent)
    : QDialog(parent),
      mainTcpSocket(socket) {
    setWindowTitle(" "); // 设置新窗口标题

    setFixedSize(600, 450); //禁用缩放

    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint); // 隐藏图标和最大化/最小化按钮，保留关闭按钮

    auto* network = NetworkManager::instance();
    //  创建并设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addStretch();

    // --- 1. 创建用户名输入行 ---
    QHBoxLayout *usernameLayout = new QHBoxLayout();
    usernameLabel = new QLabel("用户名:", this);
    usernameLineEdit = new QLineEdit(this);
    usernameLineEdit->setPlaceholderText("请输入用户名");
    usernameLineEdit->setStyleSheet("QLineEdit { "
        "border-radius: 10px; "
        "padding: 5px; border: "
        "1px solid gray; "
        "}");
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(usernameLineEdit);
    mainLayout->addLayout(usernameLayout);

    mainLayout->addSpacing(15);

    // --- 2. 创建账号输入行 ---
    QHBoxLayout *accountLayout = new QHBoxLayout();
    accountLabel = new QLabel("账   号:", this);
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
    passwordLabel = new QLabel("密   码:", this);
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

    //创建确认按钮
    confirmButton = new QPushButton("确认", this); // 按钮文本改为“确认”
    confirmButton->setStyleSheet("QPushButton {"
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
    mainLayout->addWidget(confirmButton);

    mainLayout->addStretch(); // 使布局更紧凑

    //连接信号与槽
    connect(confirmButton, &QPushButton::clicked, this, &RegisterWindow::onConfirmButtonClicked);
    connect(network, &NetworkManager::registerResponse,this, &RegisterWindow::handleRegisterResponse);
    m_blockSize = 0;
}

// 析构函数实现 (通常为空，因为 Qt 的父子对象机制会处理内存)
RegisterWindow::~RegisterWindow() {
    qDebug() << "RegisterWindow destroyed.";
}

// 重写 closeEvent 方法
void RegisterWindow::closeEvent(QCloseEvent *event) {
    // 在窗口关闭前发出信号
    emit registerWindowClosed();

    // 调用基类的 closeEvent，确保窗口正常关闭
    QDialog::closeEvent(event); // 或 QMainWindow::closeEvent(event);
}

// **主要逻辑修改：连接服务器并发送注册数据**
void RegisterWindow::onConfirmButtonClicked() {
    QString username = usernameLineEdit->text();
    QString account = accountLineEdit->text();
    QString password = passwordLineEdit->text();

    if (account.isEmpty() || password.isEmpty()) {
        // || confirmPassword.isEmpty()) { // 根据需要添加确认密码的检查
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空！");
        return;
    }

    // 使用 NetworkManager 发送注册请求
    QJsonObject request;
    request["type"] = "register";
    request["username"] = username;
    request["account"] = account;
    request["password"] = password;

    NetworkManager::instance()->sendMessage(request);

    // 禁用按钮防止重复提交
    confirmButton->setEnabled(false);
}

// 处理注册响应的槽函数
void RegisterWindow::handleRegisterResponse(const QJsonObject& response) {
    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        QMessageBox::information(this, "注册成功", message);
        this->close();
    } else {
        QMessageBox::warning(this, "注册失败", message);
    }

    // 重新启用按钮
    confirmButton->setEnabled(true);
}