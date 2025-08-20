//
// Created by 20852 on 2025/8/20.
//

#ifndef ADDFRIENDWINDOW_H
#define ADDFRIENDWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QJsonObject>

class AddFriendWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddFriendWindow(const QString& currentAccount, QWidget *parent = nullptr);
    ~AddFriendWindow();

private slots:
    void onSearchButtonClicked();
    void onAddFriendButtonClicked();
    void onSearchResponse(const QJsonObject& response);

signals:
    void friendAdded(const QString& friendAccount, const QString& friendUsername);

private:
    void setupUI();
    void sendSearchRequest(const QString& keyword);
    void sendAddFriendRequest(const QString& targetAccount);

    // UI组件
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QListWidget *searchResultList;
    QPushButton *addFriendButton;
    QLabel *statusLabel;

    // 数据
    QString currentAccount;
    QString selectedFriendAccount;
    QString selectedFriendUsername;
};

#endif // ADDFRIENDWINDOW_H