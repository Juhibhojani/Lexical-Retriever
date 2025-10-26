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


// Retrieve WordStats for a set of query words
std::vector<WordStats> TermFrequencyRepository::get_word_stats_for_query(
    const std::vector<std::string>& words)
{
    // return when there are no words or not valid connection
    std::vector<WordStats> results;
    if (!db || !db->is_connected() || words.empty()) return results;

    // Build query with IN clause
    std::string query = "SELECT tf.word, tf.doc_id, tf.word_frequency, d.total_words "
                        "FROM term_frequency tf "
                        "JOIN documents d ON tf.doc_id = d.doc_id "
                        "WHERE tf.word IN (";

    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) query += ",";
        query += "'" + words[i] + "'";
    }
    query += ");";

    PGresult* res = db->execute_query(query);
    if (!res) return results;

    int n = PQntuples(res);
    for (int i = 0; i < n; ++i) {
        results.push_back({
            PQgetvalue(res, i, 0),                     // word
            PQgetvalue(res, i, 1),                     // doc_id
            std::stoi(PQgetvalue(res, i, 2)),          // word_frequency
            std::stoi(PQgetvalue(res, i, 3))           // total_words
        });
    }

    PQclear(res);
    return results;
}

