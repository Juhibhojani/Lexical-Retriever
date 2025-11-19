#pragma once
#include "CivetServer.h"
#include "../db/connection_pool.h"
#include "../service/search_service.h"
#include "../models/idf_table.h"

class SearchController : public CivetHandler {
private:
    ConnectionPool *db_pool; // to maintain same connection object
    IDFTable* idf_table;
public:
    SearchController(ConnectionPool *db_pool, IDFTable *idf_table);

    bool handleGet(CivetServer *server, struct mg_connection *conn) override;
};
