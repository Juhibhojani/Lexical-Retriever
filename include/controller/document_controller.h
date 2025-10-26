#pragma once
#include "../service/document_service.h"
#include "CivetServer.h"
#include <string>
#include <memory>

// DocumentController is a subclass of CivetHandler which will implement get and post requests
class DocumentController : public CivetHandler {
private:
    DBConnection* db_conn; // to maintain same connection object

public:
    // Constructor that takes the DB connection pointer
    explicit DocumentController(DBConnection* conn);

    // Handle POST requests for creating a document, done via overriding default method
    bool handlePost(CivetServer* server, struct mg_connection* conn) override;

    // Handle GET request for returning document by id
    bool handleGet(CivetServer* server, struct mg_connection* conn);

    // Handle DELETE request for deleting document by id
    bool handleDelete(CivetServer* server, struct mg_connection* conn);
};
