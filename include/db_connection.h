// ensures that even if the header file is included twice then also compiler skips it for the second time
#pragma once

#include <libpq-fe.h>
#include <string>

class DBConnection
{
private:
    PGconn *conn;
    std::string connection_str;

public:
    DBConnection(const std::string &db_name,
                 const std::string &user,
                 const std::string &password,
                 const std::string &host = "127.0.0.1",
                 int port = 5432); // method to establish db connection

    ~DBConnection(); // method to disconnect db connection

    // const here signifies that the this method doesn't change any member of the class
    bool is_connected() const; // returns whether a connection exists or not to the database

    // Executes a query and returns the raw PGresult pointer. Note: to call PQclear(res) once done.
    PGresult* execute_query(const std::string &query);

    // adding a getter to obtain connection
    PGconn* get_conn() const; 

    void test_connection(); // dummy method to test connection
};

