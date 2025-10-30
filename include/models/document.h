#pragma once
#include <string>

struct Document {
    std::string doc_id;       // UUID stored as string
    std::string document_text;
    std::string created_at;   // timestamp as string
};
