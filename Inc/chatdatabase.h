#ifndef CHATDATABASE_H
#define CHATDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

struct ChatMessage {
    int id;
    QString chatType;        // "public" 或 "private"
    QString chatTarget;      // 公共聊天为"PUBLIC"，私聊为目标用户账号
    QString senderAccount;   // 发送者账号
    QString senderUsername;  // 发送者用户名
    QString content;         // 消息内容
    QDateTime timestamp;     // 时间戳
    bool isSelf;            // 是否为自己发送的消息
};

class ChatDatabase : public QObject {
    Q_OBJECT

public:
    static ChatDatabase* instance();

    // 数据库操作
    bool initDatabase(const QString& currentAccount);
    bool saveMessage(const ChatMessage& message);
    QList<ChatMessage> getMessages(const QString& chatType, const QString& chatTarget, int limit, int offset = 0);
    QList<ChatMessage> getRecentMessages(const QString& chatType, const QString& chatTarget, int limit, int offset = 0);
    bool clearMessages(const QString& chatType, const QString& chatTarget = QString());

    //获取消息总数
    int getMessageCount(const QString& chatType, const QString& chatTarget);

private:
    explicit ChatDatabase(QObject* parent = nullptr);
    QString getDatabasePath(const QString& account);

    QSqlDatabase m_database;
    QString m_currentAccount;
    static ChatDatabase* s_instance;
};

#endif // CHATDATABASE_H