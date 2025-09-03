//
// Created by 20852 on 2025/8/20.
//

#ifndef PERSONALMSGWINDOW_H
#define PERSONALMSGWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QJsonObject>

class PersonalMsgWindow : public QDialog {
    Q_OBJECT

public:
    explicit PersonalMsgWindow(const QString& username, const QString& account, QWidget *parent = nullptr);
    ~PersonalMsgWindow();

private slots:
    void setupRightWidget();
    void onAvatarButtonClicked();  // 头像按钮点击槽函数
    void onConfirmButtonClicked(); // 确认修改按钮点击槽函数
    void onUserInfoUpdateResponse(const QJsonObject& response);

signals:
    void userInfoUpdated(const QString& newUsername, const QString& account);
    void avatarChanged(const QString& account); // 头像变更信号

private:
    void setupLeftWidget();
    void setupUI();
    void sendUpdateRequest(); // 发送更新请求到服务器

    // UI组件
    QSplitter *mainSplitter;

    // 左侧头像区域
    QWidget *leftWidget;
    QPushButton *avatarButton;
    QLabel *avatarLabel;

    // 右侧信息修改区域
    QWidget *rightWidget;
    QLineEdit *nicknameEdit;
    QLineEdit *oldPasswordEdit;
    QLineEdit *newPasswordEdit;
    QPushButton *confirmButton;

    QString currentUsername; // 当前用户名
    QString currentAccount;  // 当前账号
};

#endif // PERSONALMSGWINDOW_H