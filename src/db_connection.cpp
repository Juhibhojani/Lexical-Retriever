#include "db_connection.h"
#include <iostream>

// https://www.postgresql.org/docs/current/libpq-example.html reference

// implementing the method for connecting to db
DBConnection::DBConnection(const std::string &db_name,
                           const std::string &user,
                           const std::string &password,
                           const std::string &host,
                           int port)
{
    connection_str = "dbname=" + db_name +
                     " user=" + user +
                     " password=" + password +
                     " hostaddr=" + host +
                     " port=" + std::to_string(port); // building the connection string

    // connects to db using the string and returns pointer to PGconn object                     
    conn = PQconnectdb(connection_str.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        // the connection has failed and we can't start server unless this is true
        std::cerr << "Connection failed: " << PQerrorMessage(conn);
        PQfinish(conn);
        conn = nullptr;
    }
    else
    {
        std::cout << "Connected to database: " << db_name << std::endl;
    }
}

// implementing the disconnect DB method
DBConnection::~DBConnection()
{
    if (conn)
    {
        // closes the connection
        PQfinish(conn);
        std::cout << "Connection closed." << std::endl;
    }
}

// returns whether the connection to the database is active or not
bool DBConnection::is_connected() const {
    return conn && PQstatus(conn) == CONNECTION_OK;
}


PGresult* DBConnection::execute_query(const std::string &query) {
    // check if connection is active or not
    if (!is_connected()) return nullptr;

    PGresult* res = PQexec(conn, query.c_str());
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        std::cerr << "Query failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return nullptr;
    }

    return res; // Note: To do PQclear(res) when done
}

// for internal use only
void DBConnection::test_connection() {
    if (!is_connected()) {
        std::cerr << "Not connected to DB." << std::endl;
        return;
    }

    PGresult* res = execute_query("SELECT version();");
    if (!res) {
        std::cerr << "Failed to execute test query." << std::endl;
        return;
    }

    // There should be only one row and one column
    char* version = PQgetvalue(res, 0, 0);
    std::cout << "PostgreSQL version: " << version << std::endl;

    PQclear(res);
}

// adding getter to obtain connection object
PGconn* DBConnection::get_conn() const {
    return conn;
}

bool DBConnection::begin_transaction() {
    PGresult* res = execute_query("BEGIN;");
    if (!res) return false;
    PQclear(res);
    return true;
}

bool DBConnection::commit() {
    PGresult* res = execute_query("COMMIT;");
    if (!res) return false;
    PQclear(res);
    return true;
}

bool DBConnection::rollback() {
    PGresult* res = execute_query("ROLLBACK;");
    if (!res) return false;
    PQclear(res);
    return true;
}