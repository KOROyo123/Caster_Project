#include <iostream>

#include "sqlite3.h"
#include <spdlog/spdlog.h>

#define TEST_DB_FILE_PATH "/Code/Koro_Caster/demo/sqlite_demo/data/test.db"

// typedef int (*sqlite3_callback)(void*,int,char**, char**);

// INSERT INTO user_tb (type,name,password,registertime,role,activation,activationperiod) VALUES('3','koroyo1','pwd11','2023/11/22 14:30:41','0','1','14')

//

static int select_callback(void *para, int colum, char **colum_valve, char **colum_key)
{
    for (int i = 0; i < colum; i++)
    {
        spdlog::info("{},{},{}", i, colum_key[i], colum_valve[i]);
    }
    //

    return 0;
}

int test(std::string x)
{

    int size = x.size();

    std::cout << x << size << std::endl;

    return 0;
}

int main()
{

    sqlite3 *db;

    sqlite3 *db2;

    char msg1[] = "SOURCE  KOROYO\n\r";

    char msg2[] = "SOURCE asdasdas KOROYO\n\r";

    char msg3[] = "SOURCE  KOROYO HTTP/1.1\n\r";

    char msg4[] = "SOURCE ASDASD KOROYO HTTP/1.1\n\r";

    char element1[4][128] = {'\0'};
    char element2[4][128] = {'\0'};
    char element3[4][128] = {'\0'};
    char element4[4][128] = {'\0'};

    sscanf(msg1, "%[^ |\n] %[^ |\n] %[^ |\n] %[^ |\n]", element1[0], element1[1], element1[2], element1[3]);
    sscanf(msg2, "%[^ |\n] %[^ |\n] %[^ |\n] %[^ |\n]", element2[0], element2[1], element2[2], element2[3]);
    sscanf(msg3, "%[^ |\n] %[^ |\n] %[^ |\n] %[^ |\n]", element3[0], element3[1], element3[2], element3[3]);
    sscanf(msg4, "%[^ |\n] %[^ |\n] %[^ |\n] %[^ |\n]", element4[0], element4[1], element4[2], element4[3]);

    // 打开一个数据库，不存在则自动创建
    if (sqlite3_open(TEST_DB_FILE_PATH, &db) != SQLITE_OK)
    {
        spdlog::error("sqlite:open database [{}] fail,exit!", TEST_DB_FILE_PATH);
        spdlog::error("error code: {}, msg: {} ", sqlite3_errcode(db), sqlite3_errmsg(db));
        return 1;
    }
    // 打开一个数据库，不存在则自动创建
    if (sqlite3_open(TEST_DB_FILE_PATH, &db2) != SQLITE_OK)
    {
        spdlog::error("sqlite:open database [{}] fail,exit!", TEST_DB_FILE_PATH);
        spdlog::error("error code: {}, msg: {} ", sqlite3_errcode(db), sqlite3_errmsg(db));
        return 1;
    }

    spdlog::info("sqlite:open database [{}] success!", TEST_DB_FILE_PATH);

    //
    char *errmsg;
    std::string sqlex1 = "CREATE TABLE IF NOT EXISTS userinfo(id integer primary key, name text, account varchar(255))";
    std::string sqlex2 = "INSERT INTO userinfo (name,account) VALUES ('koroyo','1121910079')";
    std::string sqlex3 = "SELECT * FROM userinfo WHERE name = 'koroyo'";
    std::string sqlex4 = "UPDATE userinfo SET name = 'nanoyo', account = '1914805725' WHERE name = 'koroyo'";
    std::string sqlex5 = "SELECT * FROM userinfo WHERE name = 'nanoyo'";
    std::string sqlex6 = "DELETE FROM userinfo WHERE name = 'nanoyo' OR  name = 'koroyo'";

    // 创建表
    sqlite3_exec(db, sqlex1.c_str(), NULL, NULL, &errmsg);

    // 插入一条数据
    sqlite3_exec(db2, sqlex2.c_str(), NULL, NULL, &errmsg);

    // 查询数据
    sqlite3_exec(db, sqlex3.c_str(), select_callback, NULL, &errmsg);

    // 更新数据
    sqlite3_exec(db2, sqlex4.c_str(), NULL, NULL, &errmsg);

    // 查询数据
    sqlite3_exec(db, sqlex5.c_str(), select_callback, NULL, &errmsg);

    // 删除数据
    sqlite3_exec(db2, sqlex6.c_str(), NULL, NULL, &errmsg);

    sqlite3_close(db);
    sqlite3_close(db2);
    spdlog::info("sqlite:close database [{}] ", TEST_DB_FILE_PATH);
    return 0;
}