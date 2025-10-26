#pragma once
#include <string>

// WordStats: used for retrieval and caching TF-IDF
struct WordStats {
    std::string word;       // The word itself
    std::string doc_id;     // Document containing the word
    int word_count;         // Count of this word in that document
    int total_words;        // Total number of words in the document
};
