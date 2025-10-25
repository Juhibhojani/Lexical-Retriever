#pragma once
#include <string>
#include <vector>
#include "db_connection.h"
#include "../models/term_frequency.h" 
#include "../models/word_stats.h"     

class TermFrequencyRepository {

private:
    DBConnection* db;

public:
    TermFrequencyRepository(DBConnection* db_conn);

    // Bulk insert/update term frequencies (Always update all words together for a single document simultaneously)
    bool insert_term_frequencies_bulk(const std::vector<TermFrequency>& term_frequencies);

    // Retrieve WordStats for a set of query words (used for TF-IDF scoring)
    std::vector<WordStats> get_word_stats_for_query(const std::vector<std::string>& words);
};
