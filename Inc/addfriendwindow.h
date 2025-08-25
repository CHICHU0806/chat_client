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
#include <QMessageBox>
#include <QJsonObject>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QTimer>

class AddFriendWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddFriendWindow(const QString& currentAccount, QWidget *parent = nullptr);
    ~AddFriendWindow();

private slots:
    void onSearchButtonClicked();
    void onAddFriendButtonClicked();
    void onSearchResponse(const QJsonObject& response);
    void onFriendRequestReceived(const QJsonObject& request);
    void onAcceptRequestClicked();
    void onRejectRequestClicked();
    void onRefreshRequestsClicked();
    void onAddFriendResponse(const QJsonObject& response);
    void onFriendRequestListReceived(const QJsonObject& response);
    void onAcceptFriendRequestResponse(const QJsonObject& response);
    void onRejectFriendRequestResponse(const QJsonObject& response);
    void onFriendRequestNotificationReceived(const QJsonObject& notification);

signals:
    void friendAdded(const QString& friendAccount, const QString& friendUsername);

private:
    void setupUI();
    void setupLeftPanel(QWidget* leftPanel);
    void setupRightPanel(QWidget* rightPanel);
    void sendSearchRequest(const QString& keyword);
    void sendAddFriendRequest(const QString& targetAccount);
    void loadFriendRequests();
    void sendAcceptFriendRequest(const QString& fromAccount);
    void sendRejectFriendRequest(const QString& fromAccount);


    // 左侧UI组件
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QListWidget *searchResultList;
    QPushButton *addFriendButton;
    QLabel *statusLabel;

    // 右侧处理好友申请的UI组件
    QListWidget *requestListWidget;
    QPushButton *acceptButton;
    QPushButton *rejectButton;
    QPushButton *refreshRequestsButton;
    QLabel *requestStatusLabel;

    // 数据
    QString currentAccount;
    QString selectedFriendAccount;
    QString selectedFriendUsername;
    QString selectedRequestAccount;      // 选中的好友请求账号
    QString selectedRequestUsername;     // 选中的好友请求用户名
};

#endif // ADDFRIENDWINDOW_H