#include "db/term_frequency_repository.h"
#include <iostream>
#include <libpq-fe.h>

// implementing the constructor
TermFrequencyRepository::TermFrequencyRepository(DBConnection* db_conn) {
    db = db_conn;
}

// Bulk insert/update term frequencies
bool TermFrequencyRepository::insert_term_frequencies_bulk(
    const std::vector<TermFrequency>& term_frequencies)
{
    // return empty in case db is not connected or the vector of term_frequencies is empty
    if (!db || !db->is_connected() || term_frequencies.empty()) return false;

    bool all_success = true;

    for (const auto& tf : term_frequencies) {
        const char* params[3] = {
            // c_str is a constant character pointer to the string object
            tf.doc_id.c_str(),
            tf.word.c_str(),
            std::to_string(tf.word_frequency).c_str()
        };

        PGresult* res = PQexecParams(db->get_conn(),
            "INSERT INTO term_frequency (doc_id, word, word_frequency) "
            "VALUES (CAST($1 AS UUID), $2, $3) "
            "ON CONFLICT (doc_id, word) DO UPDATE SET word_frequency = EXCLUDED.word_frequency;",
            3,          // number of parameters
            nullptr,   
            params,
            nullptr,    // param lengths
            nullptr,    // param formats
            0);       

        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cerr << "Insert failed for word '" << tf.word
                      << "': " << PQerrorMessage(db->get_conn()) << std::endl;
            all_success = false;
        }

        if (res) PQclear(res);
    }

    return all_success;
}

// Retrieve word vs document count 
std::vector<IDFStats> TermFrequencyRepository::get_all_idf_stats() {
    std::vector<IDFStats> results;

    // Validate DB connection
    if (!db || !db->is_connected()) return results;

    // Query to count number of documents per word
    std::string query =
        "SELECT word, COUNT(DISTINCT doc_id) AS document_count "
        "FROM term_frequency "
        "GROUP BY word;";

    // Execute query using your DB wrapper
    PGresult* res = db->execute_query(query);
    if (!res) return results;

    // Parse results
    int n = PQntuples(res);
    for (int i = 0; i < n; ++i) {
        results.push_back({
            PQgetvalue(res, i, 0),           // word
            std::stoi(PQgetvalue(res, i, 1)) // document_count
        });
    }

    PQclear(res);
    return results;
}