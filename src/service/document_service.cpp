#include "service/document_service.h"

std::optional<std::string> DocumentService::create_document(const std::string& text) {
    db_->begin_transaction();

    auto doc_id = doc_repo_->create_document(text);
    if (!doc_id) {
        db_->rollback();
        return {};
    }

    auto term_freqs = Tokenizer::tokenize_and_compute(*doc_id, text);

    if (!tf_repo_->insert_term_frequencies_bulk(term_freqs)) {
        db_->rollback();
        return {};
    }

    db_->commit();
    return doc_id;
}
