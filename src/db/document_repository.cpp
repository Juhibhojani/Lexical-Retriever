#include "db/document_repository.h"
#include <iostream>
#include <optional>
#include <vector>

using namespace std;

// constructor initialization
DocumentRepository::DocumentRepository(DBConnection* db_conn) {
    db = db_conn;
}

// CREATE
optional<string> DocumentRepository::create_document(const string &text)
{
    try
    {
        if (!db || !db->is_connected())
            return nullopt;

        const char *paramValues[1];
        paramValues[0] = text.c_str();

        string query = "INSERT INTO documents (document_text) VALUES ($1) RETURNING doc_id;";

        PGresult *res = PQexecParams(db->get_conn(),
                                     query.c_str(),
                                     1,       // number of parameters
                                     nullptr, // param types
                                     paramValues,
                                     nullptr, // param lengths
                                     nullptr, // param formats
                                     0);      // text format

        if (!res || PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            if (res)
                PQclear(res);
            cerr << "Failed to insert document: " << PQerrorMessage(db->get_conn()) << endl;
            return nullopt;
        }

        string doc_id = PQgetvalue(res, 0, 0);
        PQclear(res);
        return doc_id;
    }
    catch (const exception &e)
    {
        cerr << "Error occured at create document in repo " << e.what() << endl;
        return nullopt;
    }
}

// READ by ID
optional<Document> DocumentRepository::get_document_by_id(const string &doc_id)
{
    try
    {
        if (!db || !db->is_connected())
            return nullopt;

        string query = "SELECT doc_id, document_text, created_at FROM documents WHERE doc_id = '" + doc_id + "';";
        PGresult *res = db->execute_query(query);
        if (!res)
            return nullopt;

        if (PQntuples(res) == 0)
        {
            PQclear(res);
            return nullopt;
        }

        Document doc;
        doc.doc_id = PQgetvalue(res, 0, 0);
        doc.document_text = PQgetvalue(res, 0, 1);
        doc.created_at = PQgetvalue(res, 0, 2);

        PQclear(res);
        return doc;
    }
    catch (const exception &e)
    {
        cerr << "Error occured at get_document_by_id in repo " << e.what() << endl;
        return nullopt;
    }
}

// READ all
vector<Document> DocumentRepository::get_all_documents()
{
    vector<Document> docs;
    try
    {
        if (!db || !db->is_connected())
            return docs;

        PGresult *res = db->execute_query("SELECT doc_id, document_text, created_at FROM documents;");
        if (!res)
            return docs;

        int n = PQntuples(res);
        for (int i = 0; i < n; ++i)
        {
            docs.push_back({PQgetvalue(res, i, 0),
                            PQgetvalue(res, i, 1),
                            PQgetvalue(res, i, 2)});
        }
        PQclear(res);
    }
    catch (const exception &e)
    {
        cerr << "Error occured at get_all_documents in repo " << e.what() << endl;
    }
    return docs;
}

// Get total number of documents
int DocumentRepository::get_total_documents() {
    try
    {
        if (!db || !db->is_connected())
            return 0;

        PGresult *res = db->execute_query("SELECT COUNT(*) FROM documents;");
        if (!res)
            return 0;

        int total = stoi(PQgetvalue(res, 0, 0));
        PQclear(res);
        return total;
    }
    catch (const exception &e)
    {
        cerr << "Error occured at get_total_documents in repo " << e.what() << endl;
        return 0;
    }
}

// DELETE
bool DocumentRepository::delete_document(const string &doc_id)
{
    try
    {
        if (!db || !db->is_connected())
            return false;

        string query = "DELETE FROM documents WHERE doc_id = '" + doc_id + "';";
        PGresult *res = db->execute_query(query);
        if (!res)
            return false;

        int affected_rows = stoi(PQcmdTuples(res));
        if (affected_rows == 0)
        {
            cerr << "No document found with doc_id: " << doc_id << endl;
            PQclear(res);
            return false;
        }

        cout << "Deleted " << affected_rows << " document(s) with doc_id: " << doc_id << endl;

        PQclear(res);
        return true;
    }
    catch (const exception &e)
    {
        cerr << "Error occured at delete_document in repo " << e.what() << endl;
        return false;
    }
}
