#include "controller/search_controller.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono> // for timing

using namespace std::chrono;
using json = nlohmann::json;

// constructor to initialize the connection object
SearchController::SearchController(DBConnection* conn, IDFTable* idf)
        : db_conn(conn), idf_table(idf) {}

bool SearchController::handlePost(CivetServer* server, struct mg_connection* conn) {
    char buf[4096];
    int n = mg_read(conn, buf, sizeof(buf));
    if (n <= 0) {
        mg_printf(conn,
                  "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: 17\r\n\r\n{\"error\":\"empty\"}");
        return true;
    }

    std::string body(buf, n);
    json j;
    try { j = json::parse(body); } 
    catch (...) {
        mg_printf(conn,
                  "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: 21\r\n\r\n{\"error\":\"invalid JSON\"}");
        return true;
    }

    if (!j.contains("query")) {
        mg_printf(conn,
                  "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: 29\r\n\r\n{\"error\":\"missing query field\"}");
        return true;
    }

    std::string query = j["query"];

    // Initialize repositories and service using our db_conn
    DocumentRepository doc_repo(db_conn);
    TermFrequencyRepository tf_repo(db_conn);
    SearchService search_service(&doc_repo, &tf_repo, idf_table);

    // Record start time
    auto start = high_resolution_clock::now();

    auto results = search_service.search(query);

    // Record end time
    auto end = high_resolution_clock::now();

    // Compute duration in milliseconds
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "Execution time: " << duration.count() << std::endl;

    json j_resp;

    // check if results are empty
    if (results.empty()) {
        j_resp["results"] = json::array();
        j_resp["message"] = "No documents found";
    }
    else
    {
        json arr = json::array();
        for (auto& r : results) {
            json obj;
            obj["doc_id"] = r.doc_id;
            obj["score"] = r.score;
            obj["text"] = r.text;
            arr.push_back(obj);
        }
        j_resp["results"] = arr;
        j_resp["message"] = "Documents retrieved successfully";
        std::cout << j_resp["message"] << std::endl;
    }

    std::string response_str = j_resp.dump();

    std::cout << response_str << std::endl;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n"
              "Connection: close\r\n\r\n%s",
              response_str.size(), response_str.c_str());

    return true;
}
