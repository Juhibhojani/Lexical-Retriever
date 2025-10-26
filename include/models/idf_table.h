#pragma once
#include <string>
#include <unordered_map>
#include <pthread.h>

class IDFTable {
private:
    // storing the word : IDF in an unordered_map in-memory
    std::unordered_map<std::string, double> idf_map_;
    // mutex to ensure consistency
    pthread_mutex_t mutex_;

public:
    IDFTable();
    ~IDFTable();

    // Thread-safe setters/getters
    void set_idf(const std::string &word, double value);
    double get_idf(const std::string &word);

};
