#pragma once
#include "CivetServer.h"
#include "../db_connection.h"
#include "../service/search_service.h"
#include "../models/idf_table.h"

class SearchController : public CivetHandler {
private:
    DBConnection* db_conn; // to maintain same connection object
    IDFTable* idf_table;
public:
    SearchController(DBConnection* db_conn,IDFTable* idf_table);

    bool handlePost(CivetServer* server, struct mg_connection* conn) override;
};
