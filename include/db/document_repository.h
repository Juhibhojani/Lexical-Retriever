#pragma once
#include "./db_connection.h"
#include <string>
#include <vector>
#include <optional>
#include "../models/document.h"

class DocumentRepository {
private:
    DBConnection* db;  // pointer to DB connection

public:
    // Constructor
    DocumentRepository(DBConnection* db_conn);

    // Create a new document and return the generated doc_id
    std::optional<std::string> create_document(const std::string &text);

    // Read a document by doc_id
    std::optional<Document> get_document_by_id(const std::string &doc_id);

    // Read all documents
    std::vector<Document> get_all_documents();
    
    // Get total number of documents (for IDF calculation)
    int get_total_documents();

    // Delete a document by doc_id
    bool delete_document(const std::string &doc_id);
};
