#pragma once
#include <string>
#include <map>
#include <vector>
#include "../models/term_frequency.h" // For TermFrequency struct

class Tokenizer {
public:
    static std::vector<TermFrequency> tokenize_and_compute(const std::string& doc_id, const std::string& text);
};
