#include "personalmsgwindow.h"
    #include "networkmanager.h"
    #include <QJsonDocument>
    #include <QJsonObject>
    #include <QMessageBox>
    #include <QFileDialog>
    #include <QPixmap>

PersonalMsgWindow::PersonalMsgWindow(const QString& username, const QString& account, QWidget *parent)
    : QDialog(parent),
      mainSplitter(nullptr),
      leftWidget(nullptr),
      rightWidget(nullptr),
      avatarButton(nullptr),
      avatarLabel(nullptr),
      nicknameEdit(nullptr),
      oldPasswordEdit(nullptr),
      newPasswordEdit(nullptr),
      confirmButton(nullptr),
      currentUsername(username),
      currentAccount(account)  // 直接初始化用户信息
    {
        setupUI();
    }

    PersonalMsgWindow::~PersonalMsgWindow() {
    }

    void PersonalMsgWindow::setupUI() {
        setWindowTitle("个人信息设置");
        setFixedSize(900, 600); // 增加窗口宽度
        setModal(true);

        // 设置窗口样式 - 使用与MainWindow相同的背景色
        setStyleSheet(
            "QDialog {"
            "    background-color: #6690A0;"
            "    border-radius: 8px;"
            "}"
            "QLineEdit {"
            "    border: 1px solid #ddd;"
            "    border-radius: 6px;"
            "    padding: 10px 15px;"
            "    font-size: 14px;"
            "    background-color: white;"
            "    min-height: 20px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #1E90FF;"
            "    outline: none;"
            "}"
            "QPushButton {"
            "    background-color: #1E90FF;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 25px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    min-height: 45px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #1873CC;"
            "    transform: translateY(-1px);"
            "}"
            "QPushButton:pressed {"
            "    background-color: #0854AC;"
            "    transform: translateY(1px);"
            "}"
            "QSplitter::handle {"
            "    background-color: #5A7A8A;"
            "    width: 3px;"
            "    margin: 20px 0px;"
            "    border-radius: 1px;"
            "}"
            "QSplitter::handle:hover {"
            "    background-color: #4A6A7A;"
            "}"
        );

        // 创建主分割器
        mainSplitter = new QSplitter(Qt::Horizontal, this);
        mainSplitter->setChildrenCollapsible(false);
        mainSplitter->setHandleWidth(3);

        // 创建左侧头像区域
        setupLeftWidget();

        // 创建右侧信息修改区域
        setupRightWidget();

        // 将左右部件添加到分割器
        mainSplitter->addWidget(leftWidget);
        mainSplitter->addWidget(rightWidget);

        // 设置分割比例为1:1，让分割线在正中央
        mainSplitter->setSizes({450, 450}); // 左右各占450px，正好是900px的一半

        // 主布局
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addWidget(mainSplitter);
    }

    void PersonalMsgWindow::setupLeftWidget() {
        leftWidget = new QWidget();
        leftWidget->setStyleSheet(
        "QWidget {"
        "    background-color: transparent;" // 改为透明，使用父窗口背景
        "    border-right: none;"           // 移除右边框，由分割线代替
        "}"
    );

        QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
        leftLayout->setContentsMargins(20, 30, 20, 30);
        leftLayout->setSpacing(20);

        // 头像标签
        avatarLabel = new QLabel("头像预览");
        avatarLabel->setAlignment(Qt::AlignCenter);
        avatarLabel->setStyleSheet(
            "QLabel {"
            "    color: #666666;"
            "    font-size: 12px;"
            "    margin-bottom: 10px;"
            "}"
        );

        // 头像按钮
        avatarButton = new QPushButton();
        avatarButton->setFixedSize(120, 120);
        avatarButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #f0f0f0;"
            "    border: 2px dashed #cccccc;"
            "    border-radius: 60px;"
            "    color: #999999;"
            "    font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e8e8e8;"
            "    border-color: #1E90FF;"
            "}"
        );
        avatarButton->setText("点击选择\n头像图片");

        // 连接头像按钮信号
        connect(avatarButton, &QPushButton::clicked, this, &PersonalMsgWindow::onAvatarButtonClicked);

        leftLayout->addWidget(avatarLabel);
        leftLayout->addWidget(avatarButton, 0, Qt::AlignCenter);
        leftLayout->addStretch();
    }

    void PersonalMsgWindow::setupRightWidget() {
        rightWidget = new QWidget();
        rightWidget->setStyleSheet("QWidget { background-color: transparent; }");

        QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
        rightLayout->setContentsMargins(40, 40, 40, 40); // 增加边距
        rightLayout->setSpacing(30); // 增加间距

        // 标题
        QLabel *titleLabel = new QLabel("修改个人信息");
        titleLabel->setStyleSheet(
            "QLabel {"
            "    font-size: 20px;"  // 增加字体大小
            "    font-weight: bold;"
            "    color: #333333;"
            "    margin-bottom: 15px;"
            "}"
        );

        // 昵称行 - 标签和输入框在同一行
        QHBoxLayout *nicknameLayout = new QHBoxLayout();
        QLabel *nicknameLabel = new QLabel("昵称：");
        nicknameLabel->setStyleSheet(
            "QLabel { "
            "    color: #FFFFFF; "
            "    font-size: 15px; "
            "    font-weight: bold; "
            "    min-width: 80px; "     // 设置标签最小宽度，保持对齐
            "}"
        );

        nicknameEdit = new QLineEdit();
        nicknameEdit->setPlaceholderText("请输入新昵称");
        nicknameEdit->setText(currentUsername);
        nicknameEdit->setMinimumWidth(300); // 适当减少宽度
        nicknameEdit->setMaximumHeight(45);

        nicknameLayout->addWidget(nicknameLabel);
        nicknameLayout->addWidget(nicknameEdit);
        nicknameLayout->addStretch(); // 右侧弹性空间

        // 原密码行 - 标签和输入框在同一行
        QHBoxLayout *oldPasswordLayout = new QHBoxLayout();
        QLabel *oldPasswordLabel = new QLabel("原密码：");
        oldPasswordLabel->setStyleSheet(
            "QLabel { "
            "    color: #FFFFFF; "
            "    font-size: 15px; "
            "    font-weight: bold; "
            "    min-width: 80px; "
            "}"
        );

        oldPasswordEdit = new QLineEdit();
        oldPasswordEdit->setPlaceholderText("请输入原密码");
        oldPasswordEdit->setEchoMode(QLineEdit::Password);
        oldPasswordEdit->setMinimumWidth(300);
        oldPasswordEdit->setMaximumHeight(45);

        oldPasswordLayout->addWidget(oldPasswordLabel);
        oldPasswordLayout->addWidget(oldPasswordEdit);
        oldPasswordLayout->addStretch();

        // 新密码行 - 标签和输入框在同一行
        QHBoxLayout *newPasswordLayout = new QHBoxLayout();
        QLabel *newPasswordLabel = new QLabel("新密码：");
        newPasswordLabel->setStyleSheet(
            "QLabel { "
            "    color: #FFFFFF; "
            "    font-size: 15px; "
            "    font-weight: bold; "
            "    min-width: 80px; "
            "}"
        );

        newPasswordEdit = new QLineEdit();
        newPasswordEdit->setPlaceholderText("请输入新密码（留空则不修改）");
        newPasswordEdit->setEchoMode(QLineEdit::Password);
        newPasswordEdit->setMinimumWidth(300);
        newPasswordEdit->setMaximumHeight(45);

        newPasswordLayout->addWidget(newPasswordLabel);
        newPasswordLayout->addWidget(newPasswordEdit);
        newPasswordLayout->addStretch();

        // 确认修改按钮
        confirmButton = new QPushButton("🔄 确认修改");
        confirmButton->setMinimumHeight(55);
        confirmButton->setMinimumWidth(200);
        confirmButton->setMaximumWidth(300);
        confirmButton->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "                stop:0 #4CAF50, stop:1 #45A049);"
            "    color: white;"
            "    border: none;"
            "    border-radius: 12px;"
            "    padding: 15px 30px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    margin-top: 20px;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "                stop:0 #5CBF60, stop:1 #55B059);"
            "    transform: translateY(-2px);"
            "    box-shadow: 0 4px 8px rgba(0,0,0,0.2);"
            "}"
            "QPushButton:pressed {"
            "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "                stop:0 #3E8E41, stop:1 #357A37);"
            "    transform: translateY(1px);"
            "}"
            "QPushButton:disabled {"
            "    background-color: #CCCCCC;"
            "    color: #888888;"
            "}"
        );

        // 连接确认按钮信号
        connect(confirmButton, &QPushButton::clicked, this, &PersonalMsgWindow::onConfirmButtonClicked);

        // 添加所有组件到主布局
        rightLayout->addWidget(titleLabel);
        rightLayout->addSpacing(20);

        rightLayout->addLayout(nicknameLayout);    // 昵称行
        rightLayout->addSpacing(20);

        rightLayout->addLayout(oldPasswordLayout); // 原密码行
        rightLayout->addSpacing(20);

        rightLayout->addLayout(newPasswordLayout); // 新密码行

        rightLayout->addStretch(); // 弹性空间

        // 将按钮居中显示
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        buttonLayout->addWidget(confirmButton);
        buttonLayout->addStretch();

        rightLayout->addLayout(buttonLayout);
        rightLayout->addSpacing(20);
    }

    void PersonalMsgWindow::onAvatarButtonClicked() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "选择头像图片",
            "",
            "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"
        );

        if (!fileName.isEmpty()) {
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                // 缩放图片并设置为圆形
                QPixmap scaledPixmap = pixmap.scaled(120, 120, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                avatarButton->setIcon(QIcon(scaledPixmap));
                avatarButton->setIconSize(QSize(116, 116));
                avatarButton->setText("");
            }
        }
    }

    void PersonalMsgWindow::onConfirmButtonClicked() {
        QString nickname = nicknameEdit->text().trimmed();
        QString oldPassword = oldPasswordEdit->text();
        QString newPassword = newPasswordEdit->text();

        // 基本验证
        if (nickname.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "昵称不能为空！");
            return;
        }

        if (oldPassword.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "请输入原密码进行验证！");
            return;
        }

        // 发送更新请求
        sendUpdateRequest();
    }

    void PersonalMsgWindow::sendUpdateRequest() {
        QJsonObject request;
        request["type"] = "updateUserInfo";
        request["account"] = currentAccount;
        request["nickname"] = nicknameEdit->text().trimmed();
        request["oldPassword"] = oldPasswordEdit->text();

        // 只有在输入了新密码时才包含这个字段
        if (!newPasswordEdit->text().isEmpty()) {
            request["newPassword"] = newPasswordEdit->text();
        }

        // 发送请求
        NetworkManager::instance()->sendMessage(request);

        // 禁用按钮防止重复提交
        confirmButton->setEnabled(false);
        confirmButton->setText("修改中...");

        // 这里应该连接NetworkManager的响应信号来处理服务器回复
        // 为了简化，暂时显示一个提示
        QMessageBox::information(this, "提示", "修改请求已发送，请等待服务器响应");

        // 重新启用按钮
        confirmButton->setEnabled(true);
        confirmButton->setText("确认修改");
    }