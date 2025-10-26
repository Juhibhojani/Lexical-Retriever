#include "controller/document_controller.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// constructor to initialize the connection object
DocumentController::DocumentController(DBConnection *conn)
    : db_conn(conn) {}

// handle POST /documents
bool DocumentController::handlePost(CivetServer *server, struct mg_connection *conn)
{
    // read POST body
    const struct mg_request_info *req_info = mg_get_request_info(conn);
    long long content_len = req_info->content_length;

    std::string body;
    char buf[8192];
    long long total_read = 0;

    // this is required because of how mg_read words, i need to give it some buffer to put data into
    while (total_read < content_len)
    {
        long long to_read = std::min(content_len - total_read, (long long)sizeof(buf));
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

    std::cout << "Content length is : " << content_len << std::endl;
    // parse JSON
    json j;
    try
    {
        j = json::parse(body);
    }
    catch (const json::parse_error &e)
    {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        return true;
    }

    std::string text = j.value("text", "");

    std::cout << "text is : " << text << std::endl;

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

    std::string response = j_response.dump(); // serialize to string

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.size(),
              response.c_str());

    return true;
}

// handle GET /documents/doc_id
bool DocumentController::handleGet(CivetServer* server, mg_connection* conn) {
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    std::string uri(req_info->request_uri);

    // Expected format: /documents/<doc_id>
    std::string prefix = "/documents/";
    if (uri.find(prefix) != 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        return true;
    }

    // parses the request_uri
    std::string doc_id = uri.substr(prefix.size());
    
    // Initialize service
    DocumentRepository doc_repo(db_conn);
    TermFrequencyRepository tf_repo(db_conn);
    DocumentService service(&doc_repo, &tf_repo, db_conn);

    auto doc_opt = service.get_document_by_id(doc_id);

   if (!doc_opt) {
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

    std::string response = j_response.dump(); // serialize to string

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n\r\n%s",
              response.size(), response.c_str());

    return true;
}

// handle DELETE /documents/doc_id
bool DocumentController::handleDelete(CivetServer* server, mg_connection* conn) {
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    std::string uri(req_info->request_uri);

    // Expected format: /documents/<doc_id>
    std::string prefix = "/documents/";
    if (uri.find(prefix) != 0) {
        mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
        return true;
    }

    // parses the request_uri
    std::string doc_id = uri.substr(prefix.size());
    
    // Initialize service
    DocumentRepository doc_repo(db_conn);
    TermFrequencyRepository tf_repo(db_conn);
    DocumentService service(&doc_repo, &tf_repo, db_conn);

    bool success = service.delete_document_by_id(doc_id);

    std::cout<<"status sent: " <<success<< std::endl;
    
    // Convert document to JSON
    json j_response;
    j_response["status"] = success ? "true" : "false";
    
    std::string response = j_response.dump(); // serialize to string
    std::cout << response << std::endl;
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
