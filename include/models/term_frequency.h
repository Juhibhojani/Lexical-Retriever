#pragma once
#include <string>

struct TermFrequency {
    std::string doc_id;       // UUID stored as string
    std::string word;
    float word_frequency;
};
