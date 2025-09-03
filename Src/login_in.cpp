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
    setFixedSize(290, 420);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);

    // 主容器，用于圆角、阴影和背景色
    QWidget *mainContainer = new QWidget(this);
    mainContainer->setObjectName("mainContainer");
    mainContainer->setStyleSheet(
        "QWidget#mainContainer {"
        "background-color: #222831;"
        "border-radius: 18px;"
        "border: 2px solid #3A5A6C;"
        "}"
    );
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setXOffset(0);
    shadowEffect->setYOffset(0);
    shadowEffect->setColor(QColor(0, 0, 0, 100));
    mainContainer->setGraphicsEffect(shadowEffect);

    // 主布局，顶部边距为0，mainContainer紧贴窗口顶部和左右
    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(0, 0, 0, 0); // 四边无边距
    containerLayout->setSpacing(0); // 保证无间隙
    containerLayout->addWidget(mainContainer);

    // mainContainer 内部布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 让标题栏紧贴主容器
    mainLayout->setSpacing(0);

    // 标题栏始终在最上方
    titleBar = new QWidget(mainContainer);
    titleBar->setFixedHeight(36);
    titleBar->setStyleSheet(
        "background-color: #6690A0;"
        "border-top-left-radius: 18px;"
        "border-top-right-radius: 18px;"
        "border-bottom-left-radius: 0px;"
        "border-bottom-right-radius: 0px;"
    );
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(14, 0, 8, 0);
    QLabel *iconLabel = new QLabel(titleBar);
    iconLabel->setPixmap(QPixmap(":/icon.png").scaled(24, 24));
    QLabel *titleLabel = new QLabel("登录", titleBar);
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    QPushButton *minBtn = new QPushButton("-", titleBar);
    QPushButton *closeBtn = new QPushButton("×", titleBar);
    minBtn->setFixedSize(28, 28);
    closeBtn->setFixedSize(28, 28);
    minBtn->setFocusPolicy(Qt::NoFocus);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    minBtn->setStyleSheet(
        "QPushButton { background: none; color: white; border-radius: 14px; outline: none; }"
        "QPushButton:hover { background: #607D8B; }"
        "QPushButton:pressed { background: #455A64; }"
    );
    closeBtn->setStyleSheet(
        "QPushButton { background: none; color: white; border-radius: 14px; outline: none; }"
        "QPushButton:hover { background: #E57373; }"
        "QPushButton:pressed { background: #D32F2F; }"
    );
    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(minBtn);
    titleLayout->addWidget(closeBtn);
    mainLayout->addWidget(titleBar);

    // 顶部栏和输入区之间空隙
    mainLayout->addSpacing(110);

    // 输入区布局，留出左右边距
    QHBoxLayout *accountLayout = new QHBoxLayout();
    accountLayout->setContentsMargins(20, 0, 20, 0); // 左右边距更大
    accountLabel = new QLabel("账号:", mainContainer);
    accountLineEdit = new QLineEdit(mainContainer);
    accountLineEdit->setPlaceholderText("请输入账号");
    accountLineEdit->setStyleSheet("QLineEdit { border-radius: 10px; padding: 5px; border: 1px solid gray; }");
    accountLayout->addWidget(accountLabel);
    accountLayout->addWidget(accountLineEdit);
    mainLayout->addLayout(accountLayout);

    // 账号输入和密码输入之间空隙
    mainLayout->addSpacing(12);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->setContentsMargins(20, 0, 20, 0); // 左右边距更大
    passwordLabel = new QLabel("密码:", mainContainer);
    passwordLineEdit = new QLineEdit(mainContainer);
    passwordLineEdit->setPlaceholderText("请输入密码");
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setStyleSheet("QLineEdit { border-radius: 10px; padding: 5px; border: 1px solid gray; }");
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordLineEdit);
    mainLayout->addLayout(passwordLayout);

    // 密码输入和按钮之间空隙
    mainLayout->addSpacing(18);

    // 按钮区，按钮之间留出间距
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(20, 0, 20, 0); // 左右边距更大
    loginButton = new QPushButton("登录", mainContainer);
    loginButton->setStyleSheet("QPushButton { border-radius: 10px; background-color: #1E90FF; color: white; padding: 8px 20px; border: none; } QPushButton:hover { background-color: #4169E1; } QPushButton:pressed { background-color: #0854AC; }");
    registerButton = new QPushButton("注册", mainContainer);
    registerButton->setStyleSheet("QPushButton { border-radius: 10px; background-color: #1E90FF; color: white; padding: 8px 20px; border: none; } QPushButton:hover { background-color: #4169E1; } QPushButton:pressed { background-color: #0854AC; }");
    buttonLayout->addWidget(loginButton);
    buttonLayout->addSpacing(16); // 按钮之间间距
    buttonLayout->addWidget(registerButton);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch(); // 按钮下方留弹性空间

    meteors.clear();
    int meteorCount = 8; // 光点数量
    for (int i = 0; i < meteorCount; ++i) {
        Meteor m;
        resetMeteor(m);
        meteors.append(m);
    }
    meteorTimer = new QTimer(this);
    connect(meteorTimer, &QTimer::timeout, this, [this]{
        for (auto &m : meteors) {
            m.pos += m.vel;
            if (m.pos.x() > width() || m.pos.y() > height()) {
                resetMeteor(m);
            }
        }
        update();
    });
    meteorTimer->start(16);

    // --- 5. 连接信号与槽 ---
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterButtonClicked);
    connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    // 连接 NetworkManager 的信号
    auto* network = NetworkManager::instance();
    connect(network, &NetworkManager::loginResponse,this, &LoginWindow::handleLoginResponse);
    qDebug() << "LoginWindow initialized.";
}

LoginWindow::~LoginWindow() {
    qDebug() << "LoginWindow destroyed.";
}

void LoginWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->pos().y() <= titleBar->height()) {
        dragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void LoginWindow::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}
void LoginWindow::mouseReleaseEvent(QMouseEvent *event) {
    dragging = false;
    event->accept();
}

void LoginWindow::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    for (const auto &m : meteors) {
        QPointF head = m.pos.toPointF() + QPointF(m.length, m.length * 0.8); // 头部（右下）
        QPointF tail = m.pos.toPointF(); // 拖尾（左上）
        QLinearGradient grad(tail, head);
        grad.setColorAt(0.0, QColor(30,144,255,0));    // 拖尾透明
        grad.setColorAt(0.7, QColor(30,144,255,180));  // 中间
        grad.setColorAt(1.0, QColor(30,144,255,255));  // 头部更亮
        painter.setPen(QPen(QBrush(grad), 4));
        painter.drawLine(tail, head);
    }
}

// ButtonClicked 槽函数的实现
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

    // 调试输出：查看服务器实际返回的内容
    qDebug() << "服务器登录响应:" << response;

    // 处理登录响应
    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        QString username = response["username"].toString();
        QString account = response["account"].toString();
        if (account.isEmpty()) account = accountLineEdit->text();

        this->hide();
        try {
            if (!mainWindow) {
                mainWindow = new MainWindow(NetworkManager::instance()->getSocket(), username, account);
            }
            mainWindow->show();
        } catch (...) {
            QMessageBox::critical(this, "错误", "主界面初始化失败！");
            this->show();
        }
    } else {
        QMessageBox::warning(this, "登录失败", message);
    }
}

void LoginWindow::resetMeteor(Meteor &meteor) {
    meteor.pos = QVector2D(QRandomGenerator::global()->bounded(width()/3), QRandomGenerator::global()->bounded(height()));
    float speed = QRandomGenerator::global()->generateDouble() * (1.6f - 0.8f) + 0.8f; // 更慢
    float angle = QRandomGenerator::global()->generateDouble() * (1.0f - 0.6f) + 0.6f; // 角度偏下
    meteor.vel = QVector2D(speed, speed * angle);
    meteor.length = QRandomGenerator::global()->generateDouble() * (60.0f - 40.0f) + 40.0f;;
}
