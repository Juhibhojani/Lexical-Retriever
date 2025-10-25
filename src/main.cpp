#include "db_connection.h"
#include "db/document_repository.h"
// #include "db/term_frequency_repository.h"
#include "models/term_frequency.h"
#include "models/word_stats.h"
#include <iostream>

int main() {
    // Connect to the database
    DBConnection db_conn("lexical_retriever", "your_username", "your_password");
    if (!db_conn.is_connected()) {
        std::cerr << "DB connection failed!" << std::endl;
        return 1;
    }

    // Initialize repository
    DocumentRepository repo(&db_conn);

    // Test create_document
    auto doc_id_opt = repo.create_document("Hello world! This is a test document.");
    if (!doc_id_opt) {
        std::cerr << "Failed to create document" << std::endl;
        return 1;
    }
    std::string doc_id = *doc_id_opt;
    std::cout << "Document created with ID: " << doc_id << std::endl;

    // Test get_document_by_id
    auto doc_opt = repo.get_document_by_id(doc_id);
    if (doc_opt) {
        std::cout << "Fetched document: " << doc_opt->document_text << std::endl;
    } else {
        std::cerr << "Document not found!" << std::endl;
    }

    // Test get_all_documents
    auto all_docs = repo.get_all_documents();
    std::cout << "All documents in DB: " << std::endl;
    for (const auto& d : all_docs) {
        std::cout << " - " << d.doc_id << ": " << d.document_text << " (" << d.created_at << ")" << std::endl;
    }

    // test to get count of all documents
    int total_docs = repo.get_total_documents();
    std::cout << "Total number of documents: "  << total_docs << std::endl;


    // Test delete_document
    bool deleted = repo.delete_document(doc_id);
    std::cout << "Document deletion " << (deleted ? "succeeded" : "failed") << std::endl;

    // 7Verify deletion
    auto check_doc = repo.get_document_by_id(doc_id);
    std::cout << "After deletion, document " << (check_doc ? "still exists!" : "not found") << std::endl;

    //     // Initialize TermFrequencyRepository
    // TermFrequencyRepository tf_repo(&db_conn);

    // // Prepare bulk term frequencies for the document
    // std::vector<TermFrequency> tf_bulk = {
    //     {doc_id, "hello", 1},
    //     {doc_id, "world", 1},
    //     {doc_id, "this", 1},
    //     {doc_id, "is", 1},
    //     {doc_id, "a", 1},
    //     {doc_id, "test", 1},
    //     {doc_id, "document", 1}
    // };

    // // Insert term frequencies in bulk
    // bool inserted = tf_repo.insert_term_frequencies_bulk(tf_bulk);
    // std::cout << "Term frequencies bulk insert " << (inserted ? "succeeded" : "failed") << std::endl;

    // // Retrieve WordStats for a set of words
    // std::vector<std::string> query_words = {"hello", "test"};
    // auto word_stats = tf_repo.get_word_stats_for_query(query_words);

    // std::cout << "Retrieved WordStats for query words:" << std::endl;
    // for (const auto& ws : word_stats) {
    //     double tf = static_cast<double>(ws.word_count) / ws.total_words;
    //     std::cout << "Word: " << ws.word
    //               << ", DocID: " << ws.doc_id
    //               << ", WordCount: " << ws.word_count
    //               << ", TotalWords: " << ws.total_words
    //               << ", TF: " << tf
    //               << std::endl;
    // }


    return 0;
}

