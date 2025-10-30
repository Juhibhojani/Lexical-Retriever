#include "service/document_service.h"
#include "utils/cache_manager.h"
#include <iostream>

using namespace std;

optional<string> DocumentService::create_document(const string &text)
{
    try
    {
        // to handle multiple database inserts, using transaction to ensure atomicity
        db_->begin_transaction();

        // insert into document table
        auto doc_id = doc_repo_->create_document(text);
        if (!doc_id)
        {
            db_->rollback();
            return {};
        }

        // tokenize the text and compute term_frequency
        auto term_freqs = Tokenizer::tokenize_and_compute(*doc_id, text);

        if (!tf_repo_->insert_term_frequencies_bulk(term_freqs))
        {
            db_->rollback();
            return {};
        }

        auto &doc_cache = CacheManager::documentCache();

        // adding doc to cache
        if (doc_id.has_value())
        {
            doc_cache.put(doc_id.value(), text);
            cout << "Added to cache!" << endl;
        }

        db_->commit();
        return doc_id;
    }
    catch (const exception &e)
    {
        cerr << "Exception in create_document: " << e.what() << endl;
        return nullopt;
    }
}

optional<Document> DocumentService::get_document_by_id(const string &doc_id)
{
    try
    {
        // check if it exists in cache first
        auto &doc_cache = CacheManager::documentCache();

        // trying to retrieve from cache
        auto result = doc_cache.get(doc_id);
        if (result.has_value())
        {
            cout << "Returned from cache!" << endl;
            Document doc;
            doc.doc_id = doc_id;
            doc.document_text = result.value();
            return doc; // returns the string directly
        }
        // if it doesn't then retireve from database
        return doc_repo_->get_document_by_id(doc_id);
    }
    catch (const exception &e)
    {
        cerr << "Exception in get_document_by_id: " << e.what() << endl;
        return nullopt;
    }
}

bool DocumentService::delete_document_by_id(const string &doc_id)
{
    try
    {
        // delete it from cache if it exists
        auto &doc_cache = CacheManager::documentCache();

        bool isPresent = doc_cache.remove(doc_id);

        // if present in cache then also remove all the words in that document from cache
        if (isPresent)
        {
            auto &tf_cache = CacheManager::termFrequencyCache();
            // clearing the cache here is simpler than removing those targeted docs
            cout << "Cache cleared!" << endl;
            tf_cache.clear();
        }

        return doc_repo_->delete_document(doc_id);
    }
    catch (const exception &e)
    {
        cerr << "Exception in delete_document_by_id: " << e.what() << endl;
        return false;
    }
}
