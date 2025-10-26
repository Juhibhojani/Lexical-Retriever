#pragma once
#include <string>
#include <map>
#include <vector>
#include "../models/term_frequency.h" // For TermFrequency struct

class Tokenizer {
public:
    // Tokenize the input text into cleaned words
    static std::vector<std::string> tokenize(const std::string &text);

    // Tokenize the input and compute term frequencies
    static std::vector<TermFrequency> tokenize_and_compute(const std::string &doc_id, const std::string &text);
};
