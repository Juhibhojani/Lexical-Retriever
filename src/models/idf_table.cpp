#include "models/idf_table.h"

// constructor will initialize the mutex
IDFTable::IDFTable()
{
    pthread_mutex_init(&mutex_, nullptr);
}

// destorying mutex on deletion of IDFTable
IDFTable::~IDFTable()
{
    pthread_mutex_destroy(&mutex_);
}

double IDFTable::get_idf(const std::string &word)
{
    // acquire the lock
    pthread_mutex_lock(&mutex_);
    auto it = idf_map_.find(word);
    double result=0.0;
    if (it != idf_map_.end())
    {
        result = it->second;
    }
    pthread_mutex_unlock(&mutex_);
    return result;
}

void IDFTable::set_idf(const std::string &word, double value){
    pthread_mutex_lock(&mutex_);
    // update if words exists else add it;
    idf_map_[word]=value;
    pthread_mutex_unlock(&mutex_);
}