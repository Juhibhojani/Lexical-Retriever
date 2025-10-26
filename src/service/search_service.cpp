#include "service/search_service.h"
#include "utils/tokenizer.h"
#include <algorithm>
#include <iostream>

SearchService::SearchService(DocumentRepository* doc_repo, TermFrequencyRepository* tf_repo, IDFTable* idf_table)
    : doc_repo_(doc_repo), tf_repo_(tf_repo), idf_table_(idf_table) {}

std::vector<SearchResult> SearchService::search(const std::string& query, int top_k) {
    Tokenizer tokenizer;
    auto tokens = tokenizer.tokenize(query);

    std::vector<SearchResult> results;
    if (tokens.empty()) return results;

    // use cache here first then go to database

    // Get term frequency records for query words
    auto tf_records = tf_repo_->get_word_stats_for_query(tokens);
    if(tf_records.empty()){
        std::cout << "No TF records found for query tokens." << std::endl;
        return results;
    }

    std::cout << "TF Records fetched:" << std::endl;
    for (const auto& rec : tf_records) {
        std::cout << "doc_id: " << rec.doc_id 
                << ", word: " << rec.word 
                << ", word_frequency: " << rec.word_frequency 
                << std::endl;
    }

    // Map: doc_id -> total TF-IDF score
    std::unordered_map<std::string,double> doc_scores;

    // Precompute IDF for all query tokens
    std::unordered_map<std::string,double> idf_map;
    for (const auto& token : tokens) {
        idf_map[token] = idf_table_->get_idf(token);
    }

    // Accumulate TF-IDF scores per document
    for (const auto& rec : tf_records) {
        double tf_idf = rec.word_frequency * idf_map[rec.word];
        doc_scores[rec.doc_id] += tf_idf;
    }

    // Normalize by total number of query words
    for (auto& [doc_id, total_score] : doc_scores) {
        total_score /= tokens.size();
    }

    // Sort and get top_k - to -do : Understand this logic
    std::vector<std::pair<std::string,double>> sorted_docs(doc_scores.begin(), doc_scores.end());
    std::sort(sorted_docs.begin(), sorted_docs.end(), [](auto& a, auto& b){
        return a.second > b.second;
    });
    if (sorted_docs.size() > top_k) sorted_docs.resize(top_k);

    // use cache here first, then go to database

    // Fetch document text for top_k only
    for (auto& [doc_id, avg_score] : sorted_docs) {
        std::cout << doc_id << std::endl;
        auto doc_opt = doc_repo_->get_document_by_id(doc_id);
        std::string text = doc_opt.has_value() ? doc_opt->document_text : "";
        results.push_back({doc_id, avg_score, text});
    }
    std::cout << results.size() <<std::endl;

    return results;
}
