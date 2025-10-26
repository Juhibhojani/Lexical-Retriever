#pragma once
#include <string>

struct SearchResult {
    std::string doc_id;    // Document ID
    double score;          // Average TF-IDF score for this document
    std::string text;      // Document text 
};
