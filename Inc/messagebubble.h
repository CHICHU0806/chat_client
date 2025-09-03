//
// Created by 20852 on 2025/9/4.
//

#ifndef CHAT_CLIENT_MESSAGEBUBBLE_H
#define CHAT_CLIENT_MESSAGEBUBBLE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class MessageBubble : public QWidget {
    Q_OBJECT
public:
    explicit MessageBubble(const QString& text, const QString& senderName, const QString& timeStr, bool isSelf, QWidget* parent = nullptr);
    void setText(const QString& text);
    void setSelf(bool isSelf);
    // 可扩展：支持头像、时间戳、文件/图片等
private:
    QLabel* label;         // 消息内容
    QLabel* infoLabel;     // 昵称+时间戳
    QVBoxLayout* vLayout;  // 垂直布局
    QHBoxLayout* hLayout;  // 水平对齐
};


#endif //CHAT_CLIENT_MESSAGEBUBBLE_H