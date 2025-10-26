#include "civetweb.h"
#include <string>
#include <iostream>
#include "controller/document_controller.h"
#include "db_connection.h"
#include <cstring>

#define PORT 8080

// will remove later
// Simple test GET handler - conn is the client connection which is used to read/write HTTP data
int test_hello_handler(struct mg_connection *conn, void *cbdata) {
    const char* response = "Hello World";

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"  // response code: 200 OK
              "Content-Type: text/plain\r\n"  // type of response: text/plain
              "Content-Length: %zu\r\n"  // Length of respinse
              "\r\n%s", // black space to separate headers and then actual content
              strlen(response), response);

    return 1; // Request handled
}


int main() {
    // to-do : read values from enviornment variables
    // Connectecting to database and keeping a single connection instance throughout the application
    DBConnection db_conn("lexical_retriever", "juhi", "password");
    if (!db_conn.is_connected()) {
        std::cerr << "Failed to connect to DB" << std::endl;
        return 1;
    }

    const char *options[] = {
        "document_root", ".",   // not serving static files but required
        "listening_ports", "8080",
        nullptr
    };

    // callbacks: optional hooks (like logging, authorization). Here, we use none i.e. the empty initialization
    mg_callbacks callbacks{}; 

    // starts the CivetWeb server with the given options i.e. port 8080
    // it returns a server context
    struct mg_context *ctx = mg_start(&callbacks, nullptr, options);

    // Register handlers
    // ctx = server context, url patterns , function pointer to handle requests and callback data to give to the handler
    mg_set_request_handler(ctx, "/hello", test_hello_handler, nullptr);
    mg_set_request_handler(ctx, "/documents", DocumentController::create_document_handler, &db_conn);
    // mg_set_request_handler(ctx, "/documents/", get_document_handler, nullptr);    // We'll parse doc_id inside
    // mg_set_request_handler(ctx, "/documents/", update_document_handler, nullptr);
    // mg_set_request_handler(ctx, "/documents/", delete_document_handler, nullptr);

    // const char *options[] = {
	//     "document_root", ".", "listening_ports", PORT, 0};
    
    // // convert to cpp_options
    // std::vector<std::string> cpp_options;
    // for (int i=0; i<(sizeof(options)/sizeof(options[0])-1); i++) {
    //     cpp_options.push_back(options[i]);
    // }

	// CivetServer server(cpp_options); // <-- C++ style start

	// server.addHandler(EXAMPLE_URI, DocumentController::create_document_handler,&db_conn);

    std::cout << "Server running on port 8080...\n";
    std::cout << "Press Enter to stop.\n";
    getchar();

    // Shuts down CivetWeb cleanly, closing all connections and freeing resources
    mg_stop(ctx);
    return 0;
}
