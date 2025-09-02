//
// Created by 20852 on 2025/8/23.
//

#include "FriendListWindow.h"
#include <QMessageBox>

FriendListWindow::FriendListWindow(const QString& currentAccount, QWidget *parent)
    : QDialog(parent),
      currentAccount(currentAccount),
      friendListWidget(nullptr),
      refreshButton(nullptr),
      chatButton(nullptr),
      statusLabel(nullptr)
{
    setupUI();
    loadFriendList();
}

FriendListWindow::~FriendListWindow() {
}

void FriendListWindow::setupUI() {
    setWindowTitle("好友列表");
    setFixedSize(400, 500);
    setModal(false);

    // 设置窗口样式
    setStyleSheet(
        "QDialog {"
        "    background-color: #6690A0;"
        "    border-radius: 8px;"
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
        "QLabel {"
        "    color: white;"
        "    font-size: 12px;"
        "}"
    );

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("我的好友");
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: white;"
        "    margin-bottom: 10px;"
        "}"
    );

    // 好友列表
    friendListWidget = new QListWidget(this);
    friendListWidget->setMinimumHeight(300);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    refreshButton = new QPushButton("🔄 刷新", this);
    chatButton = new QPushButton("💬 开始聊天", this);
    chatButton->setEnabled(false);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(chatButton);

    // 状态标签
    statusLabel = new QLabel("加载好友列表中...", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(friendListWidget);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusLabel);

    // 连接信号
    connect(refreshButton, &QPushButton::clicked, this, &FriendListWindow::onRefreshButtonClicked);
    connect(chatButton, &QPushButton::clicked, this, [this]() {
        if (!selectedFriendAccount.isEmpty()) {
            emit friendSelected(selectedFriendAccount, selectedFriendUsername);
            close();
        }
    });
    connect(friendListWidget, &QListWidget::itemClicked, this, &FriendListWindow::onFriendItemClicked);
}

void FriendListWindow::onRefreshButtonClicked() {
    emit requestFriendList();
}

void FriendListWindow::onFriendItemClicked(QListWidgetItem* item) {
    if (item) {
        selectedFriendAccount = item->data(Qt::UserRole).toString();
        selectedFriendUsername = item->text().split(" - ")[0];
        chatButton->setEnabled(true);
        statusLabel->setText(QString("已选择: %1").arg(selectedFriendUsername));
    }
}

void FriendListWindow::setFriendList(const QJsonArray& friends) {
    friendListData = friends;
    loadFriendList();
}

void FriendListWindow::loadFriendList() {
    friendListWidget->clear();
    chatButton->setEnabled(false);
    if (friendListData.isEmpty()) {
        statusLabel->setText("暂无好友数据");
        QListWidgetItem* placeholderItem = new QListWidgetItem("暂无好友");
        placeholderItem->setFlags(Qt::NoItemFlags);
        placeholderItem->setForeground(QColor("#999999"));
        friendListWidget->addItem(placeholderItem);
        return;
    }
    statusLabel->setText("请选择一个好友进行聊天");
    // 展示好友列表
    for (const auto& friendValue : friendListData) {
        QJsonObject friendObj = friendValue.toObject();
        QString friendAccount = friendObj["account"].toString();
        QString friendUsername = friendObj["username"].toString();
        bool isOnline = friendObj["isOnline"].toBool();
        QString displayText = friendUsername;
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, friendAccount);
        item->setData(Qt::UserRole + 1, friendUsername);
        item->setForeground(isOnline ? QColor("#1976D2") : QColor("#888888"));
        friendListWidget->addItem(item);
    }
}