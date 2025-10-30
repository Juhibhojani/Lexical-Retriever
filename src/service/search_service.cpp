#include "service/search_service.h"
#include "utils/tokenizer.h"
#include "utils/cache_manager.h"
#include <algorithm>
#include <iostream>

using namespace std;

SearchService::SearchService(DocumentRepository* doc_repo, TermFrequencyRepository* tf_repo, IDFTable* idf_table)
    : doc_repo_(doc_repo), tf_repo_(tf_repo), idf_table_(idf_table) {}

vector<SearchResult> SearchService::search(const string &query, int top_k)
{
    vector<SearchResult> results;
    try
    {
        Tokenizer tokenizer;
        // tokenize input query
        auto tokens = tokenizer.tokenize(query);

        // if no tokens in the query, return directly
        if (tokens.empty())
            return results;

        // initialize cache
        auto &tf_cache = CacheManager::termFrequencyCache();

        vector<TermFrequency> tf_records;
        vector<string> missed_tokens;

        // checking if it exists in cache or not for each token
        for (const auto &token : tokens)
        {
            auto cached_val = tf_cache.get(token);

            // cache hit
            if (cached_val.has_value())
            {
                const auto &cached_vec = cached_val.value();
                cout << token << "found in cache" << endl;
                for (const auto &pair : cached_vec)
                {
                    // converting to required structure
                    tf_records.push_back({pair.first, token, pair.second});
                }
            }
            else // cache miss
            {
                missed_tokens.push_back(token);
            }
        }

        // add tokens into cache
        if (!missed_tokens.empty())
        {
            // query db for missed tokens
            auto db_records = tf_repo_->get_word_stats_for_query(missed_tokens);
            tf_records.insert(tf_records.end(), db_records.begin(), db_records.end());

            unordered_map<string, vector<pair<string, float>>> temp_cache;

            // convert it into required structure for cache
            for (const auto &rec : db_records)
            {
                temp_cache[rec.word].push_back(make_pair(rec.doc_id, rec.word_frequency));
            }

            // put the word into cache
            for (auto &[word, vec] : temp_cache)
            {
                tf_cache.put(word, vec);
            }
        }

        // Map: doc_id -> total TF-IDF score
        unordered_map<string, double> doc_scores;

        // Precompute IDF for all query tokens
        unordered_map<string, double> idf_map;
        for (const auto &token : tokens)
        {
            idf_map[token] = idf_table_->get_idf(token);
        }

        // Accumulate TF-IDF scores per document
        for (const auto &rec : tf_records)
        {
            double tf_idf = rec.word_frequency * idf_map[rec.word];
            doc_scores[rec.doc_id] += tf_idf;
        }

        // Normalize by total number of query words
        for (auto &[doc_id, total_score] : doc_scores)
        {
            total_score /= tokens.size();
        }

        // Sort and get top_k - to -do : Understand this logic
        vector<pair<string, double>> sorted_docs(doc_scores.begin(), doc_scores.end());
        sort(sorted_docs.begin(), sorted_docs.end(), [](auto &a, auto &b)
             { return a.second > b.second; });
        if (sorted_docs.size() > top_k)
            sorted_docs.resize(top_k);

        auto &doc_cache = CacheManager::documentCache();

        // Fetch document text for top_k only
        for (auto &[doc_id, avg_score] : sorted_docs)
        {
            string text;
            // check if it exists in cache
            auto cached_doc = doc_cache.get(doc_id);
            if (cached_doc.has_value())
            {
                text = cached_doc.value();
                cout << "While searching " << doc_id << " found in cache" << endl;
            }
            else
            {
                auto doc_opt = doc_repo_->get_document_by_id(doc_id);
                if (doc_opt.has_value())
                {
                    text = doc_opt->document_text;
                    doc_cache.put(doc_id, text);
                }
                cout << "While searching " << doc_id << " was put into cache" << endl;
            }
            results.push_back({doc_id, avg_score, text});
        }
    }
    catch (const exception &ex)
    {
        cerr << "Exception occured while searching document in search service: " << ex.what() << endl;
    }
    catch (...)
    {
        cerr << "Exception occured while searching document in search service" << endl;
    }
    return results;
}
