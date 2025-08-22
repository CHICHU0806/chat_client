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

class FriendListWindow : public QDialog {
    Q_OBJECT

public:
    explicit FriendListWindow(const QString& currentAccount, QWidget *parent = nullptr);
    ~FriendListWindow();

private slots:
    void onRefreshButtonClicked();
    void onFriendItemClicked(QListWidgetItem* item);

signals:
    void friendSelected(const QString& friendAccount, const QString& friendUsername);

private:
    void setupUI();
    void loadFriendList();

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