#include "utils/idf_updater.h"
#include "models/idf_stats.h"
#include "db/document_repository.h"
#include "db/term_frequency_repository.h"
#include "db_connection.h"
#include <unistd.h> // for sleep
#include <cmath>
#include <iostream>
#include <dotenv.h>

using namespace std;

// Function that will run inside pthread
void* idf_updater_thread(void* arg){
    try
    {

        // casting argument passed into idf_table pointer
        IDFTable *idf_table = static_cast<IDFTable *>(arg);
        dotenv::init("../../.env");

        DBConnection db_conn(dotenv::getenv("DATABASE_NAME"), dotenv::getenv("USERNAME"), dotenv::getenv("PASSWORD"));

        // initialize document repository and term_frequency repository
        DocumentRepository doc_repo_(&db_conn);
        TermFrequencyRepository term_freq_repo_(&db_conn);

        if (!db_conn.is_connected())
        {
            cerr << "Failed to connect to DB in thread\n";
            return nullptr;
        }
        cout << "Thread has been initialized" << endl;

        while (true)
        {
            cout << "Running cron job i.e. updating the IDF stats!" << endl;

            // query the term_frequency table
            // vector containing all the words
            vector<IDFStats> idf_stats = term_freq_repo_.get_all_idf_stats();

            cout << "Count of words in my system is: " << idf_stats.size() << endl;
            // query the documents table
            int total_documents = doc_repo_.get_total_documents();
            cout << "total number of documents are:" << total_documents << endl;

            // log is not defined at 0
            if (total_documents != 0)
            {
                // compute the IDF value for each word
                for (int i = 0; i < idf_stats.size(); i++)
                {
                    // store it in the idf_table
                    double idf = log(static_cast<double>(total_documents) / (idf_stats[i].document_count + 1)); // adding one to normalize the result
                    idf_table->set_idf(idf_stats[i].word, idf);
                }
            }

            cout << "IDF stats computed, will sleep now..!" << endl;
            sleep(stoi(dotenv::getenv("SLEEP_TIME")));
            // sleep
        }
    }
    catch (const exception &e)
    {
        cerr << "Exception in IDF updater thread: " << e.what() << endl;
    }
    catch (...)
    {
        cerr << "Unknown error occurred in IDF updater thread." << endl;
    }

    return nullptr;
}