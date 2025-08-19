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
    
    // 为每个用户创建独立的数据库文件
    QString dbPath = getDatabasePath(currentAccount);
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", QString("chat_db_%1").arg(currentAccount));
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qCritical() << "无法打开数据库:" << m_database.lastError().text();
        return false;
    }
    
    // 创建聊天记录表
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
    
    // 创建索引以提高查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_chat_target ON chat_messages(chat_type, chat_target, timestamp)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON chat_messages(timestamp)");
    
    qDebug() << "数据库初始化成功:" << dbPath;
    return true;
}

bool ChatDatabase::saveMessage(const ChatMessage& message) {
    if (!m_database.isOpen()) {
        qWarning() << "数据库未打开";
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO chat_messages 
        (chat_type, chat_target, sender_account, sender_username, content, timestamp, is_self)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(message.chatType);
    query.addBindValue(message.chatTarget);
    query.addBindValue(message.senderAccount);
    query.addBindValue(message.senderUsername);
    query.addBindValue(message.content);
    query.addBindValue(message.timestamp);
    query.addBindValue(message.isSelf ? 1 : 0);
    
    if (!query.exec()) {
        qWarning() << "保存消息失败:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<ChatMessage> ChatDatabase::getMessages(const QString& chatType, const QString& chatTarget, int limit) {
    QList<ChatMessage> messages;
    
    if (!m_database.isOpen()) {
        qWarning() << "数据库未打开";
        return messages;
    }
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT id, chat_type, chat_target, sender_account, sender_username, 
               content, timestamp, is_self
        FROM chat_messages 
        WHERE chat_type = ? AND chat_target = ?
        ORDER BY timestamp DESC
        LIMIT ?
    )");
    
    query.addBindValue(chatType);
    query.addBindValue(chatTarget);
    query.addBindValue(limit);
    
    if (!query.exec()) {
        qWarning() << "查询消息失败:" << query.lastError().text();
        return messages;
    }
    
    while (query.next()) {
        ChatMessage msg;
        msg.id = query.value("id").toInt();
        msg.chatType = query.value("chat_type").toString();
        msg.chatTarget = query.value("chat_target").toString();
        msg.senderAccount = query.value("sender_account").toString();
        msg.senderUsername = query.value("sender_username").toString();
        msg.content = query.value("content").toString();
        msg.timestamp = query.value("timestamp").toDateTime();
        msg.isSelf = query.value("is_self").toBool();
        
        messages.prepend(msg); // 保持时间顺序
    }
    
    return messages;
}

QList<ChatMessage> ChatDatabase::getRecentMessages(const QString& chatType, const QString& chatTarget, int limit) {
    return getMessages(chatType, chatTarget, limit);
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