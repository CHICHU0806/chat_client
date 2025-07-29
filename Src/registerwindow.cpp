//
// Created by 20852 on 25-7-22.
//
#include "registerwindow.h"
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
    QString account = accountLineEdit->text();
    QString password = passwordLineEdit->text();

    if (account.isEmpty() || password.isEmpty()) {
        // || confirmPassword.isEmpty()) { // 根据需要添加确认密码的检查
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空！");
        return;
    }

    // 确保 socket 已连接才能发送数据
    if (mainTcpSocket->state() == QAbstractSocket::ConnectedState) {
        sendRegistrationRequest(account, password);
    } else {
        QMessageBox::critical(this, "网络错误", "未连接到服务器，无法注册！");
        qWarning() << "RegisterWindow 无法发送注册请求：Socket 未连接。";
    }
}

// 辅助函数：发送注册请求到服务器
void RegisterWindow::sendRegistrationRequest(const QString &account, const QString &password) {
    QJsonObject request;
    request["type"] = "register"; // 请求类型：注册
    request["account"] = account;
    request["password"] = password;

    QByteArray jsonData = QJsonDocument(request).toJson(QJsonDocument::Compact);

    // 准备数据包：长度前缀 + JSON 数据
    QByteArray dataBlock;
    QDataStream out(&dataBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0); // 确保与服务器端版本一致
    out << (quint32) 0; // 预留总长度位置
    out << jsonData; // 写入实际 JSON 数据

    // 回到数据块开始，写入总长度
    out.device()->seek(0);
    out << (quint32) (dataBlock.size() - sizeof(quint32));

    // 发送数据
    mainTcpSocket->write(dataBlock);
    mainTcpSocket->flush(); // 强制发送缓冲数据

    qInfo() << "已发送注册请求：用户 " << account;
}
