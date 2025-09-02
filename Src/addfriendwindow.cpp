#include "addfriendwindow.h"
#include "networkmanager.h"
#include <QJsonArray>

AddFriendWindow::AddFriendWindow(const QString &currentAccount, QWidget *parent)
    : QDialog(parent),
      currentAccount(currentAccount),
      searchEdit(nullptr),
      searchButton(nullptr),
      searchResultList(nullptr),
      addFriendButton(nullptr),
      statusLabel(nullptr),
      requestListWidget(nullptr),
      acceptButton(nullptr),
      rejectButton(nullptr),
      refreshRequestsButton(nullptr),
      requestStatusLabel(nullptr) {
    setupUI();

    // 连接NetworkManager的信号
    auto *network = NetworkManager::instance();
    connect(network, &NetworkManager::searchFriendResponse, this, &AddFriendWindow::onSearchResponse);
    connect(network, &NetworkManager::friendRequestReceived, this, &AddFriendWindow::onFriendRequestReceived);
    connect(network, &NetworkManager::addFriendResponse, this, &AddFriendWindow::onAddFriendResponse);
    connect(network, &NetworkManager::friendRequestListReceived, this, &AddFriendWindow::onFriendRequestListReceived);
    connect(network, &NetworkManager::acceptFriendRequestResponse, this, &AddFriendWindow::onAcceptFriendRequestResponse);
    connect(network, &NetworkManager::rejectFriendRequestResponse, this, &AddFriendWindow::onRejectFriendRequestResponse);
    connect(network, &NetworkManager::friendRequestNotificationReceived, this, &AddFriendWindow::onFriendRequestNotificationReceived);
    QTimer::singleShot(500, this, &AddFriendWindow::loadFriendRequests);
}

AddFriendWindow::~AddFriendWindow() {
}

void AddFriendWindow::setupUI() {
    setWindowTitle("添加好友");
    setFixedSize(800, 500);
    setModal(false);

    // 设置窗口样式
    setStyleSheet(
        "QDialog {"
        "    background-color: #6690A0;"
        "    border-radius: 8px;"
        "}"
        "QLineEdit {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #1E90FF;"
        "    outline: none;"
        "}"
        "QPushButton {"
        "    background-color: #1E90FF;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1873CC;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    color: #888888;"
        "}"
        "QListWidget {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    height: 40px;"
        "    padding: 8px 12px;"
        "    border-bottom: 1px solid #F0F0F0;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #F5F5F5;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #E3F2FD;"
        "    color: #1976D2;"
        "}"
        "QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "}"
    );

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #CCCCCC;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #999999;"
        "}"
    );

    // 左侧面板
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setMinimumWidth(350);
    leftPanel->setMaximumWidth(450);
    setupLeftPanel(leftPanel);

    // 右侧面板
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setMinimumWidth(300);
    setupRightPanel(rightPanel);

    // 添加面板到分割器
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);

    // 设置分割器约束 - 参考 mainwindow.cpp
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    // 设置初始大小比例
    QList<int> sizes;
    sizes << 400 << 350; // 左右面板的初始宽度
    splitter->setSizes(sizes);

    mainLayout->addWidget(splitter);
}

void AddFriendWindow::setupLeftPanel(QWidget *leftPanel) {
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(20, 20, 20, 20);
    leftLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("搜索好友", leftPanel);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: white;"
        "    margin-bottom: 10px;"
        "}"
    );

    // 搜索输入框
    searchEdit = new QLineEdit(leftPanel);
    searchEdit->setPlaceholderText("输入账号或用户名搜索...");
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 14px;"
        "    background-color: white;"
        "    color: black;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #1E90FF;"
        "    outline: none;"
        "}"
    );

    // 搜索按钮
    searchButton = new QPushButton("搜索", leftPanel);
    searchButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #1E90FF;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1873CC;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #155BA6;"
        "}"
    );

    // 搜索结果列表
    QLabel *resultLabel = new QLabel("搜索结果:", leftPanel);
    resultLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    searchResultList = new QListWidget(leftPanel);
    searchResultList->setStyleSheet(
        "QListWidget {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    selection-background-color: #E3F2FD;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #F0F0F0;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #F5F5F5;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #E3F2FD;"
        "    color: #1976D2;"
        "}"
    );

    // 添加好友按钮
    addFriendButton = new QPushButton("发送好友申请", leftPanel);
    addFriendButton->setEnabled(false);
    addFriendButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45A049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3D8B40;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    color: #666666;"
        "}"
    );

    // 状态标签
    statusLabel = new QLabel("请输入关键词搜索好友", leftPanel);
    statusLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "    padding: 5px;"
        "    background-color: rgba(255, 255, 255, 0.1);"
        "    border-radius: 4px;"
        "}"
    );

    // 布局
    leftLayout->addWidget(titleLabel);
    leftLayout->addWidget(searchEdit);
    leftLayout->addWidget(searchButton);
    leftLayout->addWidget(resultLabel);
    leftLayout->addWidget(searchResultList);
    leftLayout->addWidget(addFriendButton);
    leftLayout->addWidget(statusLabel);
    leftLayout->addStretch();

    // 连接信号槽
    connect(searchButton, &QPushButton::clicked, this, &AddFriendWindow::onSearchButtonClicked);
    connect(addFriendButton, &QPushButton::clicked, this, &AddFriendWindow::onAddFriendButtonClicked);
    connect(searchEdit, &QLineEdit::returnPressed, this, &AddFriendWindow::onSearchButtonClicked);

    // 连接搜索结果列表选择事件
    connect(searchResultList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item) {
            QStringList data = item->data(Qt::UserRole).toStringList();
            if (data.size() >= 2) {
                selectedFriendAccount = data[0];
                selectedFriendUsername = data[1];
                addFriendButton->setEnabled(true);
                statusLabel->setText("已选择: " + selectedFriendUsername);
            }
        }
    });
}

void AddFriendWindow::setupRightPanel(QWidget *rightPanel) {
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("好友申请", rightPanel);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: white;"
        "    margin-bottom: 10px;"
        "}"
    );

    // 刷新按钮
    refreshRequestsButton = new QPushButton("刷新申请列表", rightPanel);
    refreshRequestsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F57C00;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #E65100;"
        "}"
    );

    // 申请列表标签
    QLabel *requestLabel = new QLabel("待处理申请:", rightPanel);
    requestLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");

    // 好友申请列表
    requestListWidget = new QListWidget(rightPanel);
    requestListWidget->setStyleSheet(
        "QListWidget {"
        "    border: 1px solid #E0E0E0;"
        "    border-radius: 6px;"
        "    background-color: white;"
        "    selection-background-color: #E3F2FD;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #F0F0F0;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #F5F5F5;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #E3F2FD;"
        "    color: #1976D2;"
        "}"
    );

    // 操作按钮容器
    QWidget *buttonContainer = new QWidget(rightPanel);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    // 同意按钮
    acceptButton = new QPushButton("同意", rightPanel);
    acceptButton->setEnabled(false);
    acceptButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45A049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3D8B40;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    color: #666666;"
        "}"
    );

    // 拒绝按钮
    rejectButton = new QPushButton("拒绝", rightPanel);
    rejectButton->setEnabled(false);
    rejectButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #F44336;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #D32F2F;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #B71C1C;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    color: #666666;"
        "}"
    );

    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);

    // 状态标签
    requestStatusLabel = new QLabel("请选择要处理的好友申请", rightPanel);
    requestStatusLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "    padding: 5px;"
        "    background-color: rgba(255, 255, 255, 0.1);"
        "    border-radius: 4px;"
        "}"
    );

    // 布局
    rightLayout->addWidget(titleLabel);
    rightLayout->addWidget(refreshRequestsButton);
    rightLayout->addWidget(requestLabel);
    rightLayout->addWidget(requestListWidget);
    rightLayout->addWidget(buttonContainer);
    rightLayout->addWidget(requestStatusLabel);
    rightLayout->addStretch();

    // 连接信号槽
    connect(refreshRequestsButton, &QPushButton::clicked, this, &AddFriendWindow::onRefreshRequestsClicked);
    connect(acceptButton, &QPushButton::clicked, this, &AddFriendWindow::onAcceptRequestClicked);
    connect(rejectButton, &QPushButton::clicked, this, &AddFriendWindow::onRejectRequestClicked);

    // 连接申请列表选择事件
    connect(requestListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (item) {
            QStringList data = item->data(Qt::UserRole).toStringList();
            if (data.size() >= 2) {
                selectedRequestAccount = data[0];
                selectedRequestUsername = data[1];
                acceptButton->setEnabled(true);
                rejectButton->setEnabled(true);
                requestStatusLabel->setText("已选择: " + selectedRequestUsername + " 的申请");
            }
        }
    });
}

void AddFriendWindow::onSearchButtonClicked() {
    QString keyword = searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        statusLabel->setText("请输入搜索关键词");
        return;
    }

    statusLabel->setText("搜索中...");
    searchButton->setEnabled(false);
    sendSearchRequest(keyword);
}

void AddFriendWindow::onAddFriendButtonClicked() {
    if (selectedFriendAccount.isEmpty()) {
        statusLabel->setText("请先选择要添加的用户");
        return;
    }

    statusLabel->setText("发送好友申请中...");
    addFriendButton->setEnabled(false);
    sendAddFriendRequest(selectedFriendAccount);
}

void AddFriendWindow::onSearchResponse(const QJsonObject &response) {
    qDebug() << "收到搜索响应:" << response;

    // 重置UI状态
    searchButton->setEnabled(true);
    statusLabel->setText("");

    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        // 获取用户信息
        QJsonObject userInfo = response["userInfo"].toObject();
        QString username = userInfo["username"].toString();
        QString account = userInfo["account"].toString();
        bool isOnline = userInfo["isOnline"].toBool();

        // 显示搜索结果
        searchResultList->clear();
        QListWidgetItem *item = new QListWidgetItem();
        QString displayText = QString("%1 %2 ")
                .arg(username, account);
        item->setText(displayText);
        item->setForeground(QColor(0, 0, 0)); // 黑色字体
        item->setData(Qt::UserRole, QStringList() << account << username);
        searchResultList->addItem(item);

        // 更新状态
        selectedFriendAccount = account;
        selectedFriendUsername = username;
        addFriendButton->setEnabled(true);
        statusLabel->setText("找到用户: " + username);
        statusLabel->setStyleSheet("color: green;");
    } else {
        // 搜索失败
        searchResultList->clear();
        addFriendButton->setEnabled(false);
        statusLabel->setText("搜索失败: " + message);
        statusLabel->setStyleSheet("color: red;");
    }
}

// 处理收到的好友申请
void AddFriendWindow::onFriendRequestReceived(const QJsonObject &request) {
    qDebug() << "收到好友申请通知:" << request;

    // 从服务端JSON中提取字段
    QString fromAccount = request["fromAccount"].toString();
    QString fromUsername = request["fromUsername"].toString();
    QString toAccount = request["toAccount"].toString();
    QString status = request["status"].toString(); // "onlineRequest" 或 "offlineRequest"
    QString timestamp = request["timestamp"].toString();

    // 验证这个申请是否是发给当前用户的
    if (toAccount != currentAccount) {
        qWarning() << "收到不是给当前用户的好友申请，忽略:" << toAccount << "!=" << currentAccount;
        return;
    }

    // 检查是否已经在列表中（避免重复）
    bool alreadyExists = false;
    for (int i = 0; i < requestListWidget->count(); ++i) {
        QListWidgetItem *item = requestListWidget->item(i);
        if (item) {
            QStringList data = item->data(Qt::UserRole).toStringList();
            if (data.size() > 0 && data[0] == fromAccount) {
                alreadyExists = true;
                break;
            }
        }
    }

    if (!alreadyExists) {
        // 清除占位符（如果存在）
        if (requestListWidget->count() == 1) {
            QListWidgetItem *item = requestListWidget->item(0);
            if (item && item->flags() == Qt::NoItemFlags) {
                delete requestListWidget->takeItem(0);
            }
        }

        // 添加到申请列表
        QListWidgetItem *item = new QListWidgetItem();
        QString displayText;

        if (status == "offlineRequest") {
            // 离线申请显示时间戳
            QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
            QString timeStr = dt.toString("MM-dd hh:mm");
            displayText = QString("%1 (%2) [%3]").arg(fromUsername, fromAccount, timeStr);
        } else {
            // 在线申请
            displayText = QString("%1 (%2) [新申请]").arg(fromUsername, fromAccount);
        }

        item->setText(displayText);
        item->setData(Qt::UserRole, QStringList() << fromAccount << fromUsername << timestamp);
        item->setForeground(QColor("#333333"));
        requestListWidget->addItem(item);

        qDebug() << "添加好友申请到列表:" << fromAccount << fromUsername;
    } else {
        qDebug() << "好友申请已存在，跳过重复添加:" << fromAccount;
        return;
    }

    // 更新状态标签
    int totalRequests = requestListWidget->count();
    requestStatusLabel->setText(QString("收到来自 %1 的好友申请 (共%2条)").arg(fromUsername).arg(totalRequests));
    requestStatusLabel->setStyleSheet("color: #2196F3;");

    // 只对在线申请显示弹窗通知
    if (status == "onlineRequest") {
        QMessageBox::information(this, "新的好友申请",
                                 QString("%1 (%2) 想要添加您为好友").arg(fromUsername, fromAccount));
    }
}

void AddFriendWindow::onAcceptRequestClicked() {
    if (selectedRequestAccount.isEmpty()) {
        requestStatusLabel->setText("❌ 请先选择要处理的申请");
        requestStatusLabel->setStyleSheet("color: #F44336;");
        return;
    }

    requestStatusLabel->setText("⏳ 正在接受好友申请...");
    requestStatusLabel->setStyleSheet("color: #FF9800;");

    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);

    sendAcceptFriendRequest(selectedRequestAccount);
}

void AddFriendWindow::onRejectRequestClicked() {
    if (selectedRequestAccount.isEmpty()) {
        requestStatusLabel->setText("❌ 请先选择要处理的申请");
        requestStatusLabel->setStyleSheet("color: #F44336;");
        return;
    }

    requestStatusLabel->setText("⏳ 正在拒绝好友申请...");
    requestStatusLabel->setStyleSheet("color: #FF9800;");

    // 禁用按钮防止重复点击
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);

    // 发送拒绝请求到服务器
    sendRejectFriendRequest(selectedRequestAccount);
}

void AddFriendWindow::onRefreshRequestsClicked() {
    loadFriendRequests();
}

void AddFriendWindow::onAddFriendResponse(const QJsonObject &response) {
    qDebug() << "收到添加好友响应:" << response;

    addFriendButton->setEnabled(true);

    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        QString targetAccount = response["targetAccount"].toString();
        QString targetUsername = response["targetUsername"].toString();

        statusLabel->setText("好友申请已发送给 " + targetUsername);
        statusLabel->setStyleSheet("color: green;");

        QMessageBox::information(this, "成功",
                                 QString("已向 %1 (%2) 发送好友申请").arg(targetUsername, targetAccount));

        // 清空搜索结果和选择
        searchResultList->clear();
        selectedFriendAccount.clear();
        selectedFriendUsername.clear();
        addFriendButton->setEnabled(false);

        // 发出信号通知主窗口
        emit friendAdded(targetAccount, targetUsername);
    } else {
        statusLabel->setText("发送失败: " + message);
        statusLabel->setStyleSheet("color: red;");

        QMessageBox::warning(this, "发送失败", message);
    }
}

void AddFriendWindow::onFriendRequestListReceived(const QJsonObject& response) {
    qDebug() << "收到好友申请列表响应:" << response;

    QString status = response["status"].toString();
    QString message = response["message"].toString();

    if (status == "success") {
        QJsonArray requestsArray = response["requests"].toArray();
        int requestCount = response["requestCount"].toInt();

        // 清空列表
        requestListWidget->clear();
        acceptButton->setEnabled(false);
        rejectButton->setEnabled(false);
        selectedRequestAccount.clear();
        selectedRequestUsername.clear();

        if (requestCount == 0) {
            // 没有申请时显示占位符
            QListWidgetItem *placeholderItem = new QListWidgetItem("暂无好友申请");
            placeholderItem->setFlags(Qt::NoItemFlags);
            placeholderItem->setForeground(QColor("#999999"));
            requestListWidget->addItem(placeholderItem);

            requestStatusLabel->setText("暂无待处理的好友申请");
            requestStatusLabel->setStyleSheet("color: #999999;");
        } else {
            // 添加所有申请到列表
            for (const QJsonValue& requestValue : requestsArray) {
                QJsonObject requestObj = requestValue.toObject();

                QString fromAccount = requestObj["fromAccount"].toString();
                QString fromUsername = requestObj["fromUsername"].toString();
                QString timestamp = requestObj["timestamp"].toString();

                QListWidgetItem *item = new QListWidgetItem();

                // 格式化显示文本
                QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
                QString timeStr = dt.toString("MM-dd hh:mm");
                QString displayText = QString("%1 (%2) %3 ")
                    .arg(fromUsername, fromAccount, timeStr);

                item->setText(displayText);
                item->setData(Qt::UserRole, QStringList() << fromAccount << fromUsername << timestamp);
                item->setForeground(QColor("#333333"));
                requestListWidget->addItem(item);
            }

            requestStatusLabel->setText(QString("共有 %1 个待处理申请").arg(requestCount));
            requestStatusLabel->setStyleSheet("color: #2196F3;");
        }
    } else {
        // 处理错误
        requestStatusLabel->setText("加载失败: " + message);
        requestStatusLabel->setStyleSheet("color: red;");

        // 显示错误占位符
        requestListWidget->clear();
        QListWidgetItem *errorItem = new QListWidgetItem("加载申请列表失败");
        errorItem->setFlags(Qt::NoItemFlags);
        errorItem->setForeground(QColor("#F44336"));
        requestListWidget->addItem(errorItem);

        qWarning() << "获取好友申请列表失败:" << message;
    }
}

void AddFriendWindow::onAcceptFriendRequestResponse(const QJsonObject& response) {
    qDebug() << "收到接受好友申请响应:" << response;

    QString status = response["status"].toString();
    QString message = response["message"].toString();
    QString timestamp = response["timestamp"].toString();

    // 恢复按钮状态
    acceptButton->setEnabled(true);
    rejectButton->setEnabled(true);

    if (status == "success") {
        requestStatusLabel->setText("✅ " + message);
        requestStatusLabel->setStyleSheet("color: #4CAF50; font-weight: bold;");

        // 发送信号通知主窗口更新好友列表
        emit friendAdded(selectedRequestAccount, selectedRequestUsername);

        // 刷新好友申请列表
        QTimer::singleShot(1000, this, &AddFriendWindow::loadFriendRequests);

        qInfo() << "成功接受好友申请:" << selectedRequestAccount;
    } else {
        requestStatusLabel->setText("❌ " + message);
        requestStatusLabel->setStyleSheet("color: #F44336; font-weight: bold;");

        qWarning() << "接受好友申请失败:" << message;
    }

    // 清空选中项
    selectedRequestAccount.clear();
    selectedRequestUsername.clear();
}

void AddFriendWindow::onRejectFriendRequestResponse(const QJsonObject& response) {
    qDebug() << "收到拒绝好友申请响应:" << response;

    QString status = response["status"].toString();
    QString message = response["message"].toString();
    QString timestamp = response["timestamp"].toString();

    // 恢复按钮状态
    acceptButton->setEnabled(true);
    rejectButton->setEnabled(true);

    if (status == "success") {
        requestStatusLabel->setText("✅ " + message);
        requestStatusLabel->setStyleSheet("color: #FF9800; font-weight: bold;");

        // 刷新好友申请列表
        QTimer::singleShot(1000, this, &AddFriendWindow::loadFriendRequests);

        qInfo() << "成功拒绝好友申请:" << selectedRequestAccount;
    } else {
        requestStatusLabel->setText("❌ " + message);
        requestStatusLabel->setStyleSheet("color: #F44336; font-weight: bold;");

        qWarning() << "拒绝好友申请失败:" << message;
    }

    // 清空选中项
    selectedRequestAccount.clear();
    selectedRequestUsername.clear();
}

void AddFriendWindow::onFriendRequestNotificationReceived(const QJsonObject& notification) {
    QString status = notification["status"].toString();
    QString fromAccount = notification["fromAccount"].toString();
    QString toAccount = notification["toAccount"].toString();
    QString message = notification["message"].toString();

    // 如果通知是给当前用户的
    if (fromAccount == currentAccount) {
        if (status == "accepted") {
            QMessageBox::information(this, "好友申请通过", message);
            // 发送信号更新好友列表
            emit friendAdded(toAccount, ""); // 用户名可能需要从通知中获取
        } else if (status == "rejected") {
            QMessageBox::information(this, "好友申请被拒绝", message);
        }
    }
}

void AddFriendWindow::sendSearchRequest(const QString &keyword) {
    QJsonObject request;
    request["type"] = "searchFriend";
    request["account"] = currentAccount;
    request["targetAccount"] = keyword;

    NetworkManager::instance()->sendMessage(request);
}

void AddFriendWindow::sendAddFriendRequest(const QString &targetAccount) {
    QJsonObject request;
    request["type"] = "addFriend";
    request["account"] = currentAccount;
    request["targetAccount"] = targetAccount;

    NetworkManager::instance()->sendMessage(request);

    statusLabel->setText("正在发送好友申请...");
    statusLabel->setStyleSheet("color: #FF9800;");
    addFriendButton->setEnabled(false);

    qDebug() << "发送好友申请:" << currentAccount << "->" << targetAccount;
}

void AddFriendWindow::loadFriendRequests() {
    // 清空列表
    requestListWidget->clear();
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);
    selectedRequestAccount.clear();
    selectedRequestUsername.clear();

    requestStatusLabel->setText("正在加载好友申请...");
    requestStatusLabel->setStyleSheet("color: #FF9800;");

    // 发送请求获取离线好友申请
    QJsonObject request;
    request["type"] = "getFriendRequests";
    request["account"] = currentAccount;

    NetworkManager::instance()->sendMessage(request);

    qDebug() << "请求加载好友申请列表:" << currentAccount;
}

void AddFriendWindow::sendAcceptFriendRequest(const QString& fromAccount) {
    QJsonObject request;
    request["type"] = "acceptFriendRequest";
    request["account"] = currentAccount;
    request["fromAccount"] = fromAccount;

    NetworkManager::instance()->sendMessage(request);
    qDebug() << "发送接受好友申请:" << currentAccount << "接受" << fromAccount;
}

void AddFriendWindow::sendRejectFriendRequest(const QString& fromAccount) {
    QJsonObject request;
    request["type"] = "rejectFriendRequest";
    request["account"] = currentAccount;
    request["fromAccount"] = fromAccount;

    NetworkManager::instance()->sendMessage(request);
    qDebug() << "发送拒绝好友申请:" << currentAccount << "拒绝" << fromAccount;
}