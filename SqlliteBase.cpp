#include"SqlliteBase.h"

int SqliteDemo()
{
    sqlite3 *db;
    int dbResult;

    // 打开数据库连接...
    dbResult = sqlite3_open("Users.db", &db);

    if (dbResult != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 创建表格 "Users" 并初始化表头字段和数据类型
    const char *createTableSQL = R"###(
        CREATE TABLE IF NOT EXISTS Users (
            ID INTEGER PRIMARY KEY,
            Username TEXT NOT NULL,
            Password TEXT NOT NULL,
            Email TEXT,
            RegistrationDate DATETIME,
            ChatLog TEXT
        );
    )###";

    dbResult = sqlite3_exec(db, createTableSQL, 0, 0, 0);

    if (dbResult != SQLITE_OK)
    {
        std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 关闭数据库连接
    sqlite3_close(db);

    return 0;
}
