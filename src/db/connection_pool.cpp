#include "db/connection_pool.h"
#include <iostream>

using namespace std;

ConnectionPool::ConnectionPool(int poolSize,const std::string &db_name,
                               const std::string &user,
                               const std::string &password)
    : size(poolSize)
{
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);

    try
    {
        for (size_t i = 0; i < poolSize; i++)
        {
            DBConnection *conn = new DBConnection(db_name,user,password);

            if (!conn->is_connected())
            {
                // failed → cleanup and throw
                delete conn;
                throw std::runtime_error("DB connection failed");
            }

            pool.push(conn);
        }

        cout << "Pool of database objects created: " << poolSize << endl;
    }
    catch (const std::exception &e)
    {
        // cleanup anything created so far
        while (!pool.empty())
        {
            DBConnection *c = pool.front();
            pool.pop();
            delete c;
        }

        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);

        // rethrow so caller knows this failed
        throw;
    }
}

ConnectionPool::~ConnectionPool()
{
    while (!pool.empty())
    {
        DBConnection *conn = pool.front();
        pool.pop();
        delete conn;
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}

DBConnection *ConnectionPool::acquire()
{
    pthread_mutex_lock(&lock);

    while (pool.empty())
    {
        pthread_cond_wait(&cond, &lock);
    }

    DBConnection *conn = pool.front();
    pool.pop();

    pthread_mutex_unlock(&lock);
    return conn;
}

void ConnectionPool::release(DBConnection *conn)
{
    pthread_mutex_lock(&lock);
    pool.push(conn);
    cout << "Connection released!!!!!!!" << endl;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
}
