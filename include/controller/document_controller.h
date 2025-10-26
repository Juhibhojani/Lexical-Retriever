#pragma once
#include "../service/document_service.h"
#include "civetweb.h"
#include <string>

class DocumentController {
public:
    static int create_document_handler(mg_connection* conn, void* cbdata);
};
