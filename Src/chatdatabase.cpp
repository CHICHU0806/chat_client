#include "chatdatabase.h"
#include <QDebug>
#include <QSqlRecord>

ChatDatabase* ChatDatabase::s_instance = nullptr;

ChatDatabase* ChatDatabase::instance() {
    if (!s_instance) {
        s_instance = new ChatDatabase();
    }
    return s_instance;
}

ChatDatabase::ChatDatabase(QObject* parent) : QObject(parent) {
}

QString ChatDatabase::getDatabasePath(const QString& account) {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dataPath + QString("/chat_history_%1.db").arg(account);
}

bool ChatDatabase::initDatabase(const QString& currentAccount) {
    m_currentAccount = currentAccount;

    QString dbPath = getDatabasePath(currentAccount);

    m_database = QSqlDatabase::addDatabase("QSQLITE", QString("chat_db_%1").arg(currentAccount));
    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qCritical() << "无法打开数据库:" << m_database.lastError().text();
        return false;
    }

    QSqlQuery query(m_database);
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            chat_type TEXT NOT NULL,
            chat_target TEXT NOT NULL,
            sender_account TEXT NOT NULL,
            sender_username TEXT NOT NULL,
            content TEXT NOT NULL,
            timestamp DATETIME NOT NULL,
            is_self INTEGER NOT NULL DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTableSql)) {
        qCritical() << "创建表失败:" << query.lastError().text();
        return false;
    }

    // 创建性能索引
    query.exec("CREATE INDEX IF NOT EXISTS idx_chat_target ON chat_messages(chat_type, chat_target, timestamp)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON chat_messages(timestamp)");

    // 创建防重复的复合索引
    query.exec(R"(
        CREATE UNIQUE INDEX IF NOT EXISTS idx_unique_message
        ON chat_messages(chat_type, chat_target, sender_account, content, timestamp)
    )");

    qDebug() << "数据库初始化成功:" << dbPath;
    return true;
}

bool ChatDatabase::saveMessage(const ChatMessage& message) {
    if (!m_database.isOpen()) {
        qWarning() << "数据库未打开";
        return false;
    }

    // 确保时间戳有效
    QDateTime validTimestamp = message.timestamp;
    if (!validTimestamp.isValid() || validTimestamp.isNull()) {
        validTimestamp = QDateTime::currentDateTime();
        qWarning() << "消息时间戳无效，使用当前时间:" << validTimestamp;
    }

    // 改进的重复检查逻辑 - 使用内容、发送者和时间范围检查
    QSqlQuery checkQuery(m_database);
    checkQuery.prepare(R"(
        SELECT COUNT(*) FROM chat_messages
        WHERE chat_type = ? AND chat_target = ? AND sender_account = ?
        AND content = ? AND (
            timestamp = ? OR
            ABS(strftime('%s', timestamp) - strftime('%s', ?)) < 3
        )
    )");

    checkQuery.addBindValue(message.chatType);
    checkQuery.addBindValue(message.chatTarget);
    checkQuery.addBindValue(message.senderAccount);
    checkQuery.addBindValue(message.content);
    checkQuery.addBindValue(validTimestamp.toString(Qt::ISODate));
    checkQuery.addBindValue(validTimestamp.toString(Qt::ISODate));

    if (checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if (count > 0) {
            qDebug() << "检测到重复消息，跳过保存:" << message.content.left(30);
            return true; // 返回true表示处理成功（跳过重复）
        }
    } else {
        qWarning() << "重复检查查询失败:" << checkQuery.lastError().text();
    }

    // 保存消息
    QSqlQuery insertQuery(m_database);
    insertQuery.prepare(R"(
        INSERT INTO chat_messages
        (chat_type, chat_target, sender_account, sender_username, content, timestamp, is_self)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");

    insertQuery.addBindValue(message.chatType);
    insertQuery.addBindValue(message.chatTarget);
    insertQuery.addBindValue(message.senderAccount);
    insertQuery.addBindValue(message.senderUsername);
    insertQuery.addBindValue(message.content);
    insertQuery.addBindValue(validTimestamp.toString(Qt::ISODate));
    insertQuery.addBindValue(message.isSelf ? 1 : 0);

    if (!insertQuery.exec()) {
        qWarning() << "保存消息失败:" << insertQuery.lastError().text();
        return false;
    }

    qDebug() << "消息保存成功:" << message.content.left(30);
    return true;
}

QList<ChatMessage> ChatDatabase::getMessages(const QString& chatType, const QString& chatTarget, int limit, int offset) {
    QList<ChatMessage> messages;

    QSqlQuery query(m_database);
    query.prepare("SELECT sender_account, sender_username, content, timestamp, is_self "
                  "FROM chat_messages "
                  "WHERE chat_type = ? AND chat_target = ? "
                  "ORDER BY timestamp DESC "
                  "LIMIT ? OFFSET ?");

    query.addBindValue(chatType);
    query.addBindValue(chatTarget);
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (query.exec()) {
        while (query.next()) {
            ChatMessage msg;
            msg.chatType = chatType;
            msg.chatTarget = chatTarget;
            msg.senderAccount = query.value(0).toString();
            msg.senderUsername = query.value(1).toString();
            msg.content = query.value(2).toString();
            msg.timestamp = query.value(3).toDateTime();
            msg.isSelf = query.value(4).toBool();
            messages.append(msg);
        }
        // 反转列表，使最旧的消息在前
        std::reverse(messages.begin(), messages.end());
    }

    return messages;
}
QList<ChatMessage> ChatDatabase::getRecentMessages(const QString& chatType, const QString& chatTarget, int limit, int offset) {
    return getMessages(chatType, chatTarget, limit , offset);
}

bool ChatDatabase::clearMessages(const QString& chatType, const QString& chatTarget) {
    if (!m_database.isOpen()) {
        qWarning() << "数据库未打开";
        return false;
    }
    
    QSqlQuery query(m_database);
    
    if (chatTarget.isEmpty()) {
        // 清除指定类型的所有消息
        query.prepare("DELETE FROM chat_messages WHERE chat_type = ?");
        query.addBindValue(chatType);
    } else {
        // 清除特定聊天的消息
        query.prepare("DELETE FROM chat_messages WHERE chat_type = ? AND chat_target = ?");
        query.addBindValue(chatType);
        query.addBindValue(chatTarget);
    }
    
    if (!query.exec()) {
        qWarning() << "清除消息失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool ChatDatabase::messageExists(const QString& senderAccount, const QString& content, const QDateTime& timestamp) {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM chat_messages WHERE sender_account = ? AND content = ? AND ABS(strftime('%s', timestamp) - strftime('%s', ?)) < 5");
    query.addBindValue(senderAccount);
    query.addBindValue(content);
    query.addBindValue(timestamp.toString(Qt::ISODate));

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

int ChatDatabase::getMessageCount(const QString& chatType, const QString& chatTarget) {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM chat_messages WHERE chat_type = ? AND chat_target = ?");
    query.addBindValue(chatType);
    query.addBindValue(chatTarget);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    qWarning() << "获取消息数量失败:" << query.lastError().text();
    return 0;
}