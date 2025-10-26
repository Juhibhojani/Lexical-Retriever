#include "controller/document_controller.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include <cstring>
#include <iostream>

int DocumentController::create_document_handler(mg_connection* conn, void* cbdata) {
    // casting the void pointer to appropriate datatype
    // static_cast is a way to cast the data in cpp, will be done at compile time
    DBConnection* db_conn = static_cast<DBConnection*>(cbdata);

    char buf[8192];
    int len = mg_read(conn, buf, sizeof(buf)-1);
    if (len <= 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        return 1;
    }
    buf[len] = '\0';
    std::string text(buf);

    // Initialize repositories & service
    DocumentRepository doc_repo(db_conn);
    TermFrequencyRepository tf_repo(db_conn);
    DocumentService service(&doc_repo, &tf_repo, db_conn);

    auto doc_id = service.create_document(text);
    if (!doc_id) {
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return 1;
    }

    std::string response = "Document inserted with ID: " + *doc_id;
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.size(), response.c_str());

    return 1;
}
