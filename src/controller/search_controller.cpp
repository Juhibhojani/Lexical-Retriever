#include "controller/search_controller.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono> // for timing

using namespace std;
using namespace chrono;
using json = nlohmann::json;

// constructor to initialize the connection object
SearchController::SearchController(DBConnection* conn, IDFTable* idf)
        : db_conn(conn), idf_table(idf) {}

bool SearchController::handleGet(CivetServer *server, struct mg_connection *conn)
{
    try
    {

        const struct mg_request_info *req_info = mg_get_request_info(conn);

        char query_param[1024];
        if (req_info->query_string)
        {
            // Extract "query" parameter value from query string
            if (mg_get_var(req_info->query_string, strlen(req_info->query_string),
                           "query", query_param, sizeof(query_param)) <= 0)
            {
                mg_printf(conn,
                          "HTTP/1.1 400 Bad Request\r\n"
                          "Content-Type: application/json\r\n\r\n"
                          "{\"error\":\"missing query parameter\"}");
                return true;
            }
        }
        else
        {
            mg_printf(conn,
                      "HTTP/1.1 400 Bad Request\r\n"
                      "Content-Type: application/json\r\n\r\n"
                      "{\"error\":\"no query string provided\"}");
            return true;
        }

        string query = query_param;

        // Replace '+' with space (since '+' in URLs encodes spaces)
        replace(query.begin(), query.end(), '+', ' ');

        cout << "Received query: " << query << endl;

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

        cout << "Execution time: " << duration.count() << endl;

        json j_resp;

        // check if results are empty
        if (results.empty())
        {
            j_resp["results"] = json::array();
            j_resp["message"] = "No documents found";
        }
        else
        {
            json arr = json::array();
            for (auto &r : results)
            {
                json obj;
                obj["doc_id"] = r.doc_id;
                obj["score"] = r.score;
                obj["text"] = r.text;
                arr.push_back(obj);
            }
            j_resp["results"] = arr;
            j_resp["message"] = "Documents retrieved successfully";
            cout << j_resp["message"] << endl;
        }

        string response_str = j_resp.dump();

        cout << response_str << endl;

        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n"
                  "Connection: close\r\n\r\n%s",
                  response_str.size(), response_str.c_str());

        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error handling SEARCH VIA QUERY: " << e.what() << endl;
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return true;
    }
}
