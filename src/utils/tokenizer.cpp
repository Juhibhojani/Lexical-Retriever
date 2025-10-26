#include "utils/tokenizer.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>  // for the transform function
#include <cctype> // for ::tolower and ::ispunct
#include <iostream>

// future scope: Remove stopwords

// Just tokenize input into cleaned words
std::vector<std::string> Tokenizer::tokenize(const std::string &text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string word;

    while (iss >> word) {
        // lowercase
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        // remove punctuation
        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (!word.empty())
            tokens.push_back(word);
    }
    return tokens;
}

// Tokenize input and compute term frequencies
std::vector<TermFrequency> Tokenizer::tokenize_and_compute(const std::string &doc_id, const std::string &text) {
    // map for storing only unique words
    std::unordered_map<std::string, int> word_count;
    std::vector<std::string> tokens = tokenize(text);

    for (const auto &word : tokens)
        word_count[word]++;

    int total_words = tokens.size();
    std::vector<TermFrequency> freqs;

    for (const auto &[w, count] : word_count) {
        TermFrequency tf;
        tf.doc_id = doc_id;
        tf.word = w;
        tf.word_frequency = static_cast<float>(count) / total_words;
        freqs.push_back(tf);
    }

    return freqs;
}
