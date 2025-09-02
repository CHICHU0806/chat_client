//
// Created by 20852 on 2025/8/23.
//

#ifndef FRIENDLISTWINDOW_H
#define FRIENDLISTWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QJsonObject>
#include <QJsonArray>

class FriendListWindow : public QDialog {
    Q_OBJECT

public:
    explicit FriendListWindow(const QString& currentAccount, QWidget *parent = nullptr);
    ~FriendListWindow();

    // 新增：设置好友列表数据
    void setFriendList(const QJsonArray& friends);

private slots:
    void onRefreshButtonClicked();
    void onFriendItemClicked(QListWidgetItem* item);

signals:
    void friendSelected(const QString& friendAccount, const QString& friendUsername);
    void requestFriendList();

private:
    void setupUI();
    void loadFriendList();
    QJsonArray friendListData;

    // UI组件
    QListWidget *friendListWidget;
    QPushButton *refreshButton;
    QPushButton *chatButton;
    QLabel *statusLabel;

    // 数据
    QString currentAccount;
    QString selectedFriendAccount;
    QString selectedFriendUsername;
};

#endif // FRIENDLISTWINDOW_H