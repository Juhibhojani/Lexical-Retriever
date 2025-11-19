#pragma once

#include <pthread.h>
#include <queue>
#include <string>
#include "../db_connection.h"

class ConnectionPool {
public:
    ConnectionPool(int poolSize, const std::string &db_name,
                   const std::string &user,
                   const std::string &password);
    ~ConnectionPool();

    DBConnection* acquire();
    void release(DBConnection* conn);

private:
    std::queue<DBConnection*> pool;

    pthread_mutex_t lock;
    pthread_cond_t cond;

    size_t size;
};

