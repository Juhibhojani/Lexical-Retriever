#include <string>
#include <iostream>
#include "controller/document_controller.h"
#include "db_connection.h"
#include <cstring>


#define PASSWORD password
#define USERNAME username
#define PORT 8080

int main() {
    // to-do : read values from enviornment variables
    // Connecting to database and keeping a single connection instance throughout the application
    DBConnection db_conn("lexical_retriever",USERNAME, PASSWORD);
    if (!db_conn.is_connected()) {
        std::cerr << "Failed to connect to DB" << std::endl;
        return 1;
    }

    // initializing document_handler for handling all incoming requests
    DocumentController doc_handler(&db_conn);

    std::vector<std::string> cpp_options = {
        "document_root", ".", 
        "listening_ports",  std::to_string(PORT)
    };

	CivetServer server(cpp_options);

	server.addHandler("/documents", doc_handler);

    std::cout << "Server running on port 8080...\n";
    std::cout << "Press Enter to stop.\n";
    getchar();

    return 0;
}
