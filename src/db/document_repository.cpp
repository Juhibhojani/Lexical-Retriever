#include "db/document_repository.h"
#include <iostream>
#include <optional>
#include <vector>

// constructor initialization
DocumentRepository::DocumentRepository(DBConnection* db_conn) {
    db = db_conn;
}

// CREATE
std::optional<std::string> DocumentRepository::create_document(const std::string &text) {
    if (!db || !db->is_connected()) return std::nullopt;

    const char* paramValues[1];
    paramValues[0] = text.c_str();

    std::string query = "INSERT INTO documents (document_text) VALUES ($1) RETURNING doc_id;";

    PGresult* res = PQexecParams(db->get_conn(),
                                 query.c_str(),
                                 1,       // number of parameters
                                 nullptr, // param types
                                 paramValues,
                                 nullptr, // param lengths
                                 nullptr, // param formats
                                 0);      // text format

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) PQclear(res);
        std::cerr << "Failed to insert document: " << PQerrorMessage(db->get_conn()) << std::endl;
        return std::nullopt;
    }

    std::string doc_id = PQgetvalue(res, 0, 0);
    PQclear(res);
    return doc_id;
}

// READ by ID
std::optional<Document> DocumentRepository::get_document_by_id(const std::string &doc_id) {
    if (!db || !db->is_connected()) return std::nullopt;

    std::string query = "SELECT doc_id, document_text, created_at FROM documents WHERE doc_id = '" + doc_id + "';";
    PGresult* res = db->execute_query(query);
    if (!res) return std::nullopt;

    if (PQntuples(res) == 0) {
        PQclear(res);
        return std::nullopt;
    }

    Document doc;
    doc.doc_id = PQgetvalue(res, 0, 0);
    doc.document_text = PQgetvalue(res, 0, 1);
    doc.created_at = PQgetvalue(res, 0, 2);

    PQclear(res);
    return doc;
}

// READ all
std::vector<Document> DocumentRepository::get_all_documents() {
    std::vector<Document> docs;
    if (!db || !db->is_connected()) return docs;

    PGresult* res = db->execute_query("SELECT doc_id, document_text, created_at FROM documents;");
    if (!res) return docs;

    int n = PQntuples(res);
    for (int i = 0; i < n; ++i) {
        docs.push_back({
            PQgetvalue(res, i, 0),
            PQgetvalue(res, i, 1),
            PQgetvalue(res, i, 2)
        });
    }
    PQclear(res);
    return docs;
}

// Get total number of documents
int DocumentRepository::get_total_documents() {
    if (!db || !db->is_connected()) return 0;

    PGresult* res = db->execute_query("SELECT COUNT(*) FROM documents;");
    if (!res) return 0;

    int total = std::stoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return total;
}

// DELETE
bool DocumentRepository::delete_document(const std::string &doc_id) {
    if (!db || !db->is_connected()) return false;

    std::string query = "DELETE FROM documents WHERE doc_id = '" + doc_id + "';";
    PGresult* res = db->execute_query(query);
    if (!res) return false;
    
    int affected_rows = std::stoi(PQcmdTuples(res));
    if (affected_rows == 0) {
        std::cerr << "No document found with doc_id: " << doc_id << std::endl;
        PQclear(res);
        return false;
    }

    std::cout << "Deleted " << affected_rows << " document(s) with doc_id: " << doc_id << std::endl;

    PQclear(res);
    return true;
}
