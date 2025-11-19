#include <string>
#include <iostream>
#include "controller/document_controller.h"
#include "controller/search_controller.h"
#include "db/connection_pool.h"
#include <cstring>
#include "models/idf_table.h"
#include "utils/idf_updater.h"
#include <dotenv.h>

using namespace std;

int main() {
    try
    {
        dotenv::init("../.env");

        // Initialize connection pool
        ConnectionPool *db_pool = nullptr;

        try
        {
            cout << dotenv::getenv("CONNECTION_POOL_SIZE") << endl;
            std::string pool_size_str = dotenv::getenv("CONNECTION_POOL_SIZE");
            int pool_size = std::stoi(pool_size_str);
            db_pool = new ConnectionPool(pool_size, dotenv::getenv("DATABASE_NAME"), dotenv::getenv("USERNAME"), dotenv::getenv("PASSWORD"));
        }
        catch (const std::exception &e)
        {
            cerr << "Failed to create DB connection pool: " << e.what() << endl;
            return 1;
        }

        // initializing a global idf_table which will be used everywhere
        IDFTable global_idf_table;

        // initializing thread for background processing
        pthread_t idf_thread;

        if (pthread_create(&idf_thread, nullptr, idf_updater_thread, &global_idf_table) != 0)
        {
            cerr << "Unable to start IDF updater thread\n";
            return 1;
        }

        // Detach the thread
        pthread_detach(idf_thread);

        // initializing document_handler for handling all incoming requests
        DocumentController doc_handler(db_pool);

        SearchController search_handler(db_pool, &global_idf_table);

        // can configure number of threads here.
        vector<string> cpp_options = {
            "document_root", ".",
            "listening_ports", dotenv::getenv("PORT")};

        CivetServer server(cpp_options);

        server.addHandler("/documents", doc_handler);
        server.addHandler("/search", search_handler);

        cout << "Server running on port" << dotenv::getenv("PORT") << endl;
        cout << "Press Enter to stop.\n";
        getchar();
    }
    catch (const std::exception &e)
    {
        cerr << "An exception ouccred while trying to start the server " << e.what() << endl;
        return 1;
    }

    return 0;
}
