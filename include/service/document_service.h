#pragma once
#include "../db/document_repository.h"
#include "../db/term_frequency_repository.h"
#include "../utils/tokenizer.h"
#include <string>
#include <optional>

class DocumentService {
    DocumentRepository* doc_repo_;
    TermFrequencyRepository* tf_repo_;
    DBConnection* db_;
public:
    DocumentService(DocumentRepository* doc_repo,
                    TermFrequencyRepository* tf_repo,
                    DBConnection* db)
        : doc_repo_(doc_repo), tf_repo_(tf_repo), db_(db) {}

    std::optional<std::string> create_document(const std::string& text);
};
