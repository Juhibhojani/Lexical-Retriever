#include "utils/tokenizer.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>  // for the transform function
#include <cctype> // for ::tolower and ::ispunct
#include <iostream>
#include <unordered_set>

using namespace std;

// For stopwords removal
static const unordered_set<string> STOPWORDS = {
    "a", "an", "the", "and", "or", "but",
    "if", "while", "with", "to", "of", "for", "in", "on", "at", "by",
    "from", "as", "is", "are", "was", "were", "be", "been", "being",
    "it", "this", "that", "these", "those", "i", "you", "he", "she",
    "they", "we", "me", "him", "her", "them", "my", "your", "his",
    "their", "our", "so", "because", "what", "which", "who", "whom"};

// Just tokenize input into cleaned words
vector<string> Tokenizer::tokenize(const string &text)
{
    vector<string> tokens;
    try
    {
        istringstream iss(text);
        string word;

        while (iss >> word)
        {
            // lowercase
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            // remove punctuation
            word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
            if (!word.empty() && STOPWORDS.find(word) == STOPWORDS.end())
            {
                tokens.push_back(word);
            }
        }
        return tokens;
    }
    catch (const exception &e)
    {
        cerr << "Error while tokenzing input: " << e.what() << endl;
    }
}

// Tokenize input and compute term frequencies
vector<TermFrequency> Tokenizer::tokenize_and_compute(const string &doc_id, const string &text)
{
    try
    {
        // map for storing only unique words
        unordered_map<string, int> word_count;
        vector<string> tokens = tokenize(text);

        for (const auto &word : tokens)
            word_count[word]++;

        int total_words = tokens.size();
        vector<TermFrequency> freqs;

        for (const auto &[w, count] : word_count)
        {
            TermFrequency tf;
            tf.doc_id = doc_id;
            tf.word = w;
            tf.word_frequency = static_cast<float>(count) / total_words;
            freqs.push_back(tf);
        }
        return freqs;
    }
    catch (const exception &e)
    {
        cerr << "Error while performing tokenize and compute function: " << e.what() << endl;
    }
}
