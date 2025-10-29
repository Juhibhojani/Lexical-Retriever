#pragma once

#include <string>
#include <vector>
#include <utility>
#include "utils/lru_cache.h"
#include <dotenv.h>
#include <iostream>

// global singleton instance will be created
// to-do: put values are configurable

class CacheManager
{
public:
    // Initializes dotenv ONCE
    static void init()
    {
        static bool initialized = false;
        if (!initialized)
        {
            dotenv::init("../../.env");
            initialized = true;
        }
    }

    // Singleton for term frequency cache
    static LRUCache<std::string, std::vector<std::pair<std::string, float>>> &termFrequencyCache()
    {
        static LRUCache<std::string, std::vector<std::pair<std::string, float>>> tf_cache(std::stoi(dotenv::getenv("TERM_FREQUENCY_CACHE_SIZE")));
        return tf_cache;
    }

    // Singleton for document cache
    static LRUCache<std::string, std::string> &documentCache()
    {
        static LRUCache<std::string, std::string> doc_cache(std::stoi(dotenv::getenv("DOCUMENT_CACHE_SIZE")));
        return doc_cache;
    }

private:
    // Prevent external construction
    CacheManager() = default;
};
