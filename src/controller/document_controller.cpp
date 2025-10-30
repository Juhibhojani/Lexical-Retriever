#include "controller/document_controller.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include <chrono> // for timing

using namespace std;
using json = nlohmann::json;
using namespace chrono;

// constructor to initialize the connection object
DocumentController::DocumentController(DBConnection *conn)
    : db_conn(conn) {}

// handle POST /documents
bool DocumentController::handlePost(CivetServer *server, struct mg_connection *conn)
{
    try
    {
        // read POST body
        const struct mg_request_info *req_info = mg_get_request_info(conn);
        long long content_len = req_info->content_length;

        string body;
        char buf[8192];
        long long total_read = 0;

        // this is required because of how mg_read words, i need to give it some buffer to put data into
        while (total_read < content_len)
        {
            long long to_read = min(content_len - total_read, (long long)sizeof(buf));
            int n = mg_read(conn, buf, static_cast<size_t>(to_read));
            if (n <= 0)
            {
                // Error or client closed connection
                mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                return true;
            }
            body.append(buf, n);
            total_read += n;
        }

        cout << "Content length is : " << content_len << endl;
        // parse JSON
        json j;
        j = json::parse(body);
        string text = j.value("text", "");

        cout << "text is : " << text << endl;

        // Initialize repositories and service using our db_conn
        DocumentRepository doc_repo(db_conn);
        TermFrequencyRepository tf_repo(db_conn);
        DocumentService service(&doc_repo, &tf_repo, db_conn);

        // Create document - calling service which will handle business logic
        auto doc_id = service.create_document(text);
        if (!doc_id)
        {
            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
            return true;
        }

        // Prepare JSON response
        json j_response;
        j_response["status"] = "success";
        j_response["document_id"] = *doc_id;

        string response = j_response.dump(); // serialize to string

        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  response.size(),
                  response.c_str());

        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error handling CREATE DOCUMENT: " << e.what() << endl;
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return true;
    }
}

// handle GET /documents/doc_id
bool DocumentController::handleGet(CivetServer* server, mg_connection* conn) {
    try
    {
        const struct mg_request_info *req_info = mg_get_request_info(conn);
        string uri(req_info->request_uri);

        // Expected format: /documents/<doc_id>
        string prefix = "/documents/";
        if (uri.find(prefix) != 0)
        {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            return true;
        }

        // parses the request_uri
        string doc_id = uri.substr(prefix.size());

        // Initialize service
        DocumentRepository doc_repo(db_conn);
        TermFrequencyRepository tf_repo(db_conn);
        DocumentService service(&doc_repo, &tf_repo, db_conn);

        // Record start time
        auto start = high_resolution_clock::now();

        auto doc_opt = service.get_document_by_id(doc_id);

        // Record end time
        auto end = high_resolution_clock::now();

        // Compute duration in milliseconds
        auto duration = duration_cast<milliseconds>(end - start);

        cout << "Execution time: " << duration.count() << endl;

        if (!doc_opt)
        {
            mg_printf(conn,
                      "HTTP/1.1 404 Not Found\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %zu\r\n\r\n"
                      "{}",
                      static_cast<size_t>(2)); // empty JSON
            return true;
        }

        // Convert document to JSON
        json j_response;
        j_response["doc_id"] = doc_opt->doc_id;
        j_response["text"] = doc_opt->document_text;

        string response = j_response.dump(); // serialize to string

        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  response.size(), response.c_str());

        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error handling SEARCH VIA DOC_ID: " << e.what() << endl;
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return true;
    }
}

// handle DELETE /documents/doc_id
bool DocumentController::handleDelete(CivetServer* server, mg_connection* conn) {
    try
    {

        const struct mg_request_info *req_info = mg_get_request_info(conn);
        string uri(req_info->request_uri);

        // Expected format: /documents/<doc_id>
        string prefix = "/documents/";
        if (uri.find(prefix) != 0)
        {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            return true;
        }

    // parses the request_uri
    string doc_id = uri.substr(prefix.size());

    // Initialize service
    DocumentRepository doc_repo(db_conn);
    TermFrequencyRepository tf_repo(db_conn);
    DocumentService service(&doc_repo, &tf_repo, db_conn);

    bool success = service.delete_document_by_id(doc_id);

    cout << "status sent: " << success << endl;

    // Convert document to JSON
    json j_response;
    j_response["status"] = success ? "true" : "false";

    string response = j_response.dump(); // serialize to string
    cout << response << endl;
    if (!success) {
        // Document not found or delete failed
        mg_printf(conn,
                  "HTTP/1.1 404 Not Found\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %zu\r\n\r\n%s",
                  response.size(), response.c_str());
        return true;
    }

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.size(), response.c_str());

    return true;
    }
    catch (const exception &e)
    {
        cerr << "Error handling DELETING DOCUMENT: " << e.what() << endl;
        mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return true;
    }
}
