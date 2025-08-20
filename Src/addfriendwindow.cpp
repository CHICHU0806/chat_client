//
// Created by 20852 on 2025/8/20.
//

#include "addfriendwindow.h"
#include "networkmanager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidgetItem>

AddFriendWindow::AddFriendWindow(const QString& currentAccount, QWidget *parent)
    : QDialog(parent), currentAccount(currentAccount)
{
    setupUI();

    // 连接网络管理器信号
    connect(NetworkManager::instance(), &NetworkManager::searchFriendResponse,
            this, &AddFriendWindow::onSearchResponse);
}

AddFriendWindow::~AddFriendWindow() {
}

void AddFriendWindow::setupUI() {
    setWindowTitle("添加好友");
    setFixedSize(400, 480);
    setModal(true);

    setStyleSheet(
        "QDialog {"
        "    background-color: #6690A0;"
        "    border-radius: 10px;"
        "}"
        "QLineEdit {"
        "    border: 2px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    padding: 12px 15px;"
        "    font-size: 14px;"
        "    background-color: #FFFFFF;"
        "    color: #333333;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #4CAF50;"
        "    background-color: #FAFAFA;"
        "}"
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5CBF60;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3E8E41;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #CCCCCC;"
        "    color: #888888;"
        "}"
        "QListWidget {"
        "    border: 2px solid #E0E0E0;"
        "    border-radius: 8px;"
        "    background-color: #FFFFFF;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    padding: 12px;"
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
        "    color: #FFFFFF;"
        "    font-size: 14px;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 标题
    QLabel *titleLabel = new QLabel("搜索并添加好友");
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    color: #FFFFFF;"
        "    margin-bottom: 10px;"
        "}"
    );

    // 搜索区域
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("输入用户名或账号进行搜索...");
    searchEdit->setMinimumHeight(45);

    searchButton = new QPushButton("🔍 搜索");
    searchButton->setMinimumWidth(80);
    searchButton->setMinimumHeight(40);

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    // 搜索结果列表
    QLabel *resultLabel = new QLabel("搜索结果：");
    searchResultList = new QListWidget();
    searchResultList->setMinimumHeight(210);

    // 添加好友按钮
    addFriendButton = new QPushButton("➕ 添加好友");
    addFriendButton->setMinimumHeight(45);
    addFriendButton->setEnabled(false);

    // 状态标签
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("QLabel { color: #FFD700; font-weight: bold; }");

    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(searchResultList);
    mainLayout->addWidget(addFriendButton);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();

    // 连接信号
    connect(searchButton, &QPushButton::clicked, this, &AddFriendWindow::onSearchButtonClicked);
    connect(searchEdit, &QLineEdit::returnPressed, this, &AddFriendWindow::onSearchButtonClicked);
    connect(addFriendButton, &QPushButton::clicked, this, &AddFriendWindow::onAddFriendButtonClicked);
    connect(searchResultList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item) {
            selectedFriendAccount = item->data(Qt::UserRole).toString();
            selectedFriendUsername = item->text().split(" - ")[0];
            addFriendButton->setEnabled(true);
            statusLabel->setText(QString("已选择: %1").arg(selectedFriendUsername));
        }
    });
}

void AddFriendWindow::onSearchButtonClicked() {
    QString keyword = searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入搜索关键词！");
        return;
    }

    searchButton->setEnabled(false);
    searchButton->setText("⏳ 搜索中...");
    statusLabel->setText("正在搜索...");
    searchResultList->clear();
    addFriendButton->setEnabled(false);

    sendSearchRequest(keyword);
}

void AddFriendWindow::onAddFriendButtonClicked() {
    if (selectedFriendAccount.isEmpty()) {
        QMessageBox::warning(this, "选择错误", "请先选择要添加的好友！");
        return;
    }

    int ret = QMessageBox::question(this, "确认添加",
                                   QString("确定要添加 %1 为好友吗？").arg(selectedFriendUsername),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        addFriendButton->setEnabled(false);
        addFriendButton->setText("⏳ 添加中...");
        sendAddFriendRequest(selectedFriendAccount);
    }
}

void AddFriendWindow::onSearchResponse(const QJsonObject& response) {
    // 重新启用搜索按钮
    searchButton->setEnabled(true);
    searchButton->setText("🔍 搜索");

    QString status = response["status"].toString();
    if (status == "success") {
        QJsonArray users = response["users"].toArray();
        if (users.isEmpty()) {
            statusLabel->setText("未找到匹配的用户");
            return;
        }

        searchResultList->clear();
        for (const auto& userValue : users) {
            QJsonObject user = userValue.toObject();
            QString account = user["account"].toString();
            QString username = user["username"].toString();

            // 不显示自己
            if (account == currentAccount) {
                continue;
            }

            QListWidgetItem* item = new QListWidgetItem(QString("%1 - %2").arg(username, account));
            item->setData(Qt::UserRole, account);
            searchResultList->addItem(item);
        }

        if (searchResultList->count() == 0) {
            statusLabel->setText("未找到其他用户");
        } else {
            statusLabel->setText(QString("找到 %1 个用户").arg(searchResultList->count()));
        }
    } else {
        QString message = response["message"].toString();
        statusLabel->setText("搜索失败: " + message);
    }
}

void AddFriendWindow::sendSearchRequest(const QString& keyword) {
    QJsonObject request;
    request["type"] = "searchFriend";
    request["account"] = currentAccount;
    request["targetAccount"] = keyword;  // 改为targetAccount，表示要搜索的目标账号

    NetworkManager::instance()->sendMessage(request);
}

void AddFriendWindow::sendAddFriendRequest(const QString& targetAccount) {
    QJsonObject request;
    request["type"] = "addFriend";
    request["account"] = currentAccount;
    request["targetAccount"] = targetAccount;

    NetworkManager::instance()->sendMessage(request);
}