#pragma once
#include <string>

struct Document {
    std::string doc_id;       // UUID stored as string
    std::string document_text;
    std::string created_at;   // timestamp as string
};

// post  => tokenization, insert into cache and DB 
// get, delete,
// get by id

// cache implementations: Document cache and Term cache

// background thread processing - timer based update. 
// global IDF table - in-memory