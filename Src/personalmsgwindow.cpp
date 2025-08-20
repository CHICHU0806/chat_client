//
// Created by 20852 on 2025/8/20.
//

#include "personalmsgwindow.h"

PersonalMsgWindow::PersonalMsgWindow(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

PersonalMsgWindow::~PersonalMsgWindow() {
}

void PersonalMsgWindow::setupUI() {
    setWindowTitle("个人消息");
    setFixedSize(600, 450);
    setModal(true); // 设置为模态窗口

    // 设置窗口样式
    setStyleSheet(
        "QDialog {"
        "    background-color: #F5F5F5;"
        "    border-radius: 8px;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 添加一个占位标签
    QLabel *placeholderLabel = new QLabel("个人消息窗口内容待开发...", this);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 14px;"
        "    padding: 50px;"
        "}"
    );

    mainLayout->addWidget(placeholderLabel);
}