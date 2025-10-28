#include "service/document_service.h"
#include "utils/cache_manager.h"
#include <iostream>

std::optional<std::string> DocumentService::create_document(const std::string& text) {
    // to handle multiple database inserts, using transaction to ensure atomicity
    db_->begin_transaction();

    // insert into document table 
    auto doc_id = doc_repo_->create_document(text);
    if (!doc_id) {
        db_->rollback();
        return {};
    }

    // tokenize the text and compute term_frequency
    auto term_freqs = Tokenizer::tokenize_and_compute(*doc_id, text);

    if (!tf_repo_->insert_term_frequencies_bulk(term_freqs)) {
        db_->rollback();
        return {};
    }

    // to-do : use cache and add documents in cache simultanoeusly
    auto &doc_cache = CacheManager::documentCache();

    // adding doc to cache
    if (doc_id.has_value())
    {
        doc_cache.put(doc_id.value(), text);
        std::cout << "Added to cache!" << std::endl;
    }

    db_->commit();
    return doc_id;
}

std::optional<Document> DocumentService::get_document_by_id(const std::string& doc_id){
    // check if it exists in cache first
    auto &doc_cache = CacheManager::documentCache();

    // trying to retrieve from cache
    auto result = doc_cache.get(doc_id);
    if (result.has_value())
    {
        std::cout << "Returned from cache!" << std::endl;
        Document doc;
        doc.doc_id = doc_id;
        doc.document_text = result.value();
        return doc; // returns the string directly
    }
    // if it doesn't then retireve from database
    return doc_repo_->get_document_by_id(doc_id);
}

bool DocumentService::delete_document_by_id(const std::string& doc_id){
    // delete it from cache if it exists
    auto &doc_cache = CacheManager::documentCache();

    bool isPresent = doc_cache.remove(doc_id);

    // if present in cache then also remove all the words in that document from cache
    if (isPresent)
    {
        auto &tf_cache = CacheManager::termFrequencyCache();
        // clearing the cache here is simpler than removing those targeted docs
        std::cout << "Cache cleared!" << std::endl;
        tf_cache.clear();
    }

    return doc_repo_->delete_document(doc_id);
}

