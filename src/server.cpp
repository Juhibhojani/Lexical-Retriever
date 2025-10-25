#include "civetweb.h"
#include <string>
#include <iostream>

// Forward declarations of handlers
int create_document_handler(struct mg_connection *conn, void *cbdata);
int get_document_handler(struct mg_connection *conn, void *cbdata);
int update_document_handler(struct mg_connection *conn, void *cbdata);
int delete_document_handler(struct mg_connection *conn, void *cbdata);

// Simple test GET handler
int test_hello_handler(struct mg_connection *conn, void *cbdata) {
    const char* response = "Hello World";

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %zu\r\n"
              "\r\n%s",
              strlen(response), response);

    return 1; // Request handled
}


int main() {
    const char *options[] = {
        "document_root", ".",   // not serving static files but required
        "listening_ports", "8080",
        nullptr
    };

    // callbacks: optional hooks (like logging, authorization). Here, we use none.
    struct mg_callbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));

    // starts the CivetWeb server with the given options i.e. port 8080
    struct mg_context *ctx = mg_start(&callbacks, nullptr, options);

    // Register handlers
    // ctx = server context, url patterns , function pointer to handle requests 
    mg_set_request_handler(ctx, "/hello", test_hello_handler, nullptr);
    mg_set_request_handler(ctx, "/documents", create_document_handler, nullptr);
    mg_set_request_handler(ctx, "/documents/", get_document_handler, nullptr);    // We'll parse doc_id inside
    mg_set_request_handler(ctx, "/documents/", update_document_handler, nullptr);
    mg_set_request_handler(ctx, "/documents/", delete_document_handler, nullptr);

    std::cout << "Server running on port 8080...\n";
    std::cout << "Press Enter to stop.\n";
    getchar();

    mg_stop(ctx);
    return 0;
}
