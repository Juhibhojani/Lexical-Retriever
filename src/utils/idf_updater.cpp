#include "utils/idf_updater.h"
#include "models/idf_stats.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include "db_connection.h"
#include <unistd.h> // for sleep
#include <cmath>
#include <iostream>
#include <dotenv.h>

// sleep takes seconds as input, let's run it every 15 mins i.e. 15*60
#define SLEEP_TIME 100

// Function that will run inside pthread
void* idf_updater_thread(void* arg){
    // casting argument passed into idf_table pointer
    IDFTable* idf_table = static_cast<IDFTable*>(arg);
    dotenv::init("../../.env");

    DBConnection db_conn(dotenv::getenv("DATABASE_NAME"), dotenv::getenv("USERNAME"), dotenv::getenv("PASSWORD"));

    // initialize document repository and term_frequency repository
    DocumentRepository doc_repo_(&db_conn);
    TermFrequencyRepository term_freq_repo_(&db_conn);

    if (!db_conn.is_connected()) {
        std::cerr << "Failed to connect to DB in thread\n";
        return nullptr;
    }
    std::cout << "Thread has been initialized" << std::endl;

    while(true){
        std::cout << "Running cron job again!" << std::endl;

        // query the term_frequency table
        // vector containing all the words
        std::vector<IDFStats> idf_stats=term_freq_repo_.get_all_idf_stats();

        std::cout << "Count of words in my system is: " << idf_stats.size() << std::endl;
        // query the documents table
        int total_documents = doc_repo_.get_total_documents();
        std::cout << "total number of documents are:" << total_documents << std::endl;

        // compute the IDF value for each word
        for(int i=0;i<idf_stats.size();i++){
            // store it in the idf_table
            double idf = log(static_cast<double>(total_documents)/idf_stats[i].document_count+1); // adding one to normalize the result
            idf_table->set_idf(idf_stats[i].word,idf);
        }

        std::cout << "Done sleeping now!" << std::endl;
        sleep(std::stoi(dotenv::getenv("SLEEP_TIME")));
        // sleep
    }

    return nullptr;
}