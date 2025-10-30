#include "models/idf_table.h"
#include <iostream>

using namespace std;

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

double IDFTable::get_idf(const string &word)
{
    // acquire the lock
    pthread_mutex_lock(&mutex_);
    double result = 0.0;
    try
    {
        auto it = idf_map_.find(word);
        if (it != idf_map_.end())
        {
            result = it->second;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error while getting value from IDF " << e.what() << endl;
    }
    pthread_mutex_unlock(&mutex_);
    return result;
}

void IDFTable::set_idf(const string &word, double value)
{
    pthread_mutex_lock(&mutex_);
    try
    {
        // update if words exists else add it;
        idf_map_[word] = value;
    }
    catch (const exception &e)
    {
        cerr << "Error in setting IDF:" << e.what() << endl;
    }
    pthread_mutex_unlock(&mutex_);
}