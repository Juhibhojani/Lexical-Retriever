#include "service/document_service.h"

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

    // to-do : use cache and add documents in case simultanoeusly

    db_->commit();
    return doc_id;
}

std::optional<Document> DocumentService::get_document_by_id(const std::string& doc_id){
    // check if it exists in cache first
    
    // if it doesn't then retireve from database
    return doc_repo_->get_document_by_id(doc_id);
}

bool DocumentService::delete_document_by_id(const std::string& doc_id){
    // delete it from cache if it exists

    return doc_repo_->delete_document(doc_id);
}

