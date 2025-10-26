#include <string>
#include <iostream>
#include "controller/document_controller.h"
#include "db_connection.h"
#include <cstring>
#include "models/idf_table.h"
#include "utils/idf_updater.h"

#define PASSWORD "password"
#define USERNAME "juhi"
#define PORT 8080

int main() {
    // to-do : read values from enviornment variables
    // Connecting to database and keeping a single connection instance throughout the application
    DBConnection db_conn("lexical_retriever",USERNAME, PASSWORD);
    if (!db_conn.is_connected()) {
        std::cerr << "Failed to connect to DB" << std::endl;
        return 1;
    }

    // initializing a global idf_table which will be used everywhere
    IDFTable global_idf_table;

    // initializing thread for background processing
    pthread_t idf_thread;

    if (pthread_create(&idf_thread, nullptr, idf_updater_thread, &global_idf_table) != 0) {
        std::cerr <<"Unable to start IDF updater thread\n";
        return 1;
    }

    // Detach the thread
    pthread_detach(idf_thread);

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
