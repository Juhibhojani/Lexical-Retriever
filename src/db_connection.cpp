#include "db_connection.h"
#include <iostream>

using namespace std;
// https://www.postgresql.org/docs/current/libpq-example.html reference

// implementing the method for connecting to db
DBConnection::DBConnection(const string &db_name,
                           const string &user,
                           const string &password,
                           const string &host,
                           int port)
{
    connection_str = "dbname=" + db_name +
                     " user=" + user +
                     " password=" + password +
                     " hostaddr=" + host +
                     " port=" + to_string(port); // building the connection string

    // connects to db using the string and returns pointer to PGconn object                     
    conn = PQconnectdb(connection_str.c_str());

    if (PQstatus(conn) != CONNECTION_OK)
    {
        // the connection has failed and we can't start server unless this is true
        cerr << "Connection failed: " << PQerrorMessage(conn);
        PQfinish(conn);
        conn = nullptr;
    }
    else
    {
        cout << "Connected to database: " << db_name << endl;
    }
}

// implementing the disconnect DB method
DBConnection::~DBConnection()
{
    if (conn)
    {
        // closes the connection
        PQfinish(conn);
        cout << "Connection closed." << endl;
    }
}

// returns whether the connection to the database is active or not
bool DBConnection::is_connected() const {
    return conn && PQstatus(conn) == CONNECTION_OK;
}

PGresult *DBConnection::execute_query(const string &query)
{
    // check if connection is active or not
    if (!is_connected()) return nullptr;

    PGresult* res = PQexec(conn, query.c_str());
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        cerr << "Query failed: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return nullptr;
    }

    return res; // Note: To do PQclear(res) when done
}

// for internal use only
void DBConnection::test_connection() {
    if (!is_connected()) {
        cerr << "Not connected to DB." << endl;
        return;
    }

    PGresult* res = execute_query("SELECT version();");
    if (!res) {
        cerr << "Failed to execute test query." << endl;
        return;
    }

    // There should be only one row and one column
    char* version = PQgetvalue(res, 0, 0);
    cout << "PostgreSQL version: " << version << endl;

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