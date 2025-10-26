#pragma once
#include "../db/document_repository.h"
#include "../db/term_frequency_repository.h"
#include "../utils/tokenizer.h"
#include "../models/document.h"
#include "../models/idf_table.h"
#include "../models/search_result.h"
#include <string>
#include <optional>

class SearchService
{
    DocumentRepository *doc_repo_;
    TermFrequencyRepository *tf_repo_;
    IDFTable *idf_table_;

public:
    SearchService(DocumentRepository *doc_repo,TermFrequencyRepository *tf_repo,IDFTable *idf_table);

    std::vector<SearchResult> search(const std::string& query, int top_k=3);
};
