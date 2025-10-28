#pragma once

#include <string>
#include <vector>
#include <utility>
#include "utils/lru_cache.h"

// global singleton instance will be created
// to-do: put values are configurable

class CacheManager
{
public:
    // Singleton for term frequency cache
    static LRUCache<std::string, std::vector<std::pair<std::string, float>>> &termFrequencyCache()
    {
        static LRUCache<std::string, std::vector<std::pair<std::string, float>>> tf_cache(5000);
        return tf_cache;
    }

    // Singleton for document cache
    static LRUCache<std::string, std::string> &documentCache()
    {
        static LRUCache<std::string, std::string> doc_cache(2000);
        return doc_cache;
    }

private:
    // Prevent external construction
    CacheManager() = default;
};
