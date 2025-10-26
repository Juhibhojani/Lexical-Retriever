#include "utils/tokenizer.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>

std::vector<TermFrequency> Tokenizer::tokenize_and_compute(const std::string& doc_id, const std::string& text) {
    std::unordered_map<std::string, int> word_count;
    std::istringstream iss(text);
    std::string word;
    
    // Simple tokenization: split by space
    while (iss >> word) {
        // lowercase
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        // remove punctuation (optional)
        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());

        if (!word.empty()) word_count[word]++;
    }

    int total_words = text.empty() ? 1 : word_count.size();

    std::vector<TermFrequency> freqs;
    for (auto& [w, count] : word_count) {
        TermFrequency tf;
        tf.doc_id = doc_id;
        tf.word = w;
        tf.word_frequency = static_cast<float>(count) / total_words;
        freqs.push_back(tf);
    }

    return freqs;
}
