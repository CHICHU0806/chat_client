//
// Created by 20852 on 2025/9/4.
//

#include "../Inc/messagebubble.h"

MessageBubble::MessageBubble(const QString& text, const QString& senderName, const QString& timeStr, bool isSelf, QWidget* parent)
    : QWidget(parent)
{
    infoLabel = new QLabel(QString("%1  %2").arg(senderName, timeStr), this);
    infoLabel->setStyleSheet("color:#666;font-size:11px;margin-bottom:2px;");
    label = new QLabel(text, this);
    label->setWordWrap(true);
    label->setStyleSheet(isSelf ?
        "background:#1E90FF;color:white;border-radius:18px;padding:10px 18px;font-size:14px;" :
        "background:#F0F0F0;color:#333;border-radius:18px;padding:10px 18px;font-size:14px;");
    vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(2);
    vLayout->addWidget(infoLabel);
    vLayout->addWidget(label);
    hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0,0,0,0);
    if (isSelf) {
        hLayout->addStretch();
        hLayout->addLayout(vLayout);
    } else {
        hLayout->addLayout(vLayout);
        hLayout->addStretch();
    }
    setLayout(hLayout);
    setMinimumWidth(200);
    setMaximumWidth(800);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void MessageBubble::setText(const QString& text) {
    label->setText(text);
}

void MessageBubble::setSelf(bool isSelf) {
    hLayout->removeItem(vLayout);
    label->setStyleSheet(isSelf ?
        "background:#1E90FF;color:white;border-radius:18px;padding:10px 18px;font-size:14px;" :
        "background:#F0F0F0;color:#333;border-radius:18px;padding:10px 18px;font-size:14px;");
    if (isSelf) {
        hLayout->addStretch();
        hLayout->addLayout(vLayout);
    } else {
        hLayout->addLayout(vLayout);
        hLayout->addStretch();
    }
}
