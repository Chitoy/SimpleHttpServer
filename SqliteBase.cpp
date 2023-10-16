#include"SqliteBase.h"

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
            ChatLog TEXT"
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

int SqlDemo2() {
    sqlite3* db;
    int dbResult;

    // 打开数据库连接...
    dbResult = sqlite3_open("library.db", &db);

    if (dbResult != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 创建 "Authors" 表格
    const char* createAuthorsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Authors (
            AuthorID INTEGER PRIMARY KEY,
            Name TEXT NOT NULL
        );
    )";

    dbResult = sqlite3_exec(db, createAuthorsTableSQL, 0, 0, 0);

    if (dbResult != SQLITE_OK) {
        std::cerr << "Error creating Authors table: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 创建 "Books" 表格，并添加外键约束
    const char* createBooksTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Books (
            BookID INTEGER PRIMARY KEY,
            Title TEXT NOT NULL,
            AuthorID INTEGER,
            FOREIGN KEY (AuthorID) REFERENCES Authors(AuthorID)
        );
    )";

    dbResult = sqlite3_exec(db, createBooksTableSQL, 0, 0, 0);

    if (dbResult != SQLITE_OK) {
        std::cerr << "Error creating Books table: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 插入数据
    const char* insertAuthorSQL = "INSERT INTO Authors (Name) VALUES ('J.K. Rowling');";
    dbResult = sqlite3_exec(db, insertAuthorSQL, 0, 0, 0);

    if (dbResult != SQLITE_OK) {
        std::cerr << "Error inserting author: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    const char* insertBookSQL = "INSERT INTO Books (Title, AuthorID) VALUES ('Harry Potter', 1);";
    dbResult = sqlite3_exec(db, insertBookSQL, 0, 0, 0);

    if (dbResult != SQLITE_OK) {
        std::cerr << "Error inserting book: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 关闭数据库连接
    sqlite3_close(db);

    return 0;
}

