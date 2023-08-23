#pragma once

#include <mysql/mysql.h>

#include <iostream>
#include <list>

#include "locker.h"

class connection_pool {
 public:
    connection_pool();
    ~connection_pool();

    static connection_pool *getInstance();  // 单例模式

    void init(string url, int port, string user, string passwd, string dbName, int maxConn);

    MYSQL *getConn();
    bool releaseConn(MYSQL *conn);  // 释放连接
    int getFreeConnCount();
    void destoryPool();

 private:
    unsigned int maxConn;
    unsigned int currConn;
    unsigned int freeConn;

 private:
    locker lock;
    sem reserve;  // 储备
    list<MYSQL *> connList;

 private:
    string url;
    string port;
    string user;
    string passwd;
    string dbName;
};

// 在类的构造函数中请求资源，在类的析构函数中释放资源的技术称为资源获取即初始化
class connectionRAII {
 public:
    connectionRAII(MYSQL **conn, connection_pool *connPool);
    ~connectionRAII();

 private:
    MYSQL *conRAII;
    connection_pool *poolRAII;
};
