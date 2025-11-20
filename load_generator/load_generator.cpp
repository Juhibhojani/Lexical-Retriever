// LOAD GENERATOR FOR GET REQUESTS

#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <chrono>
#include <cstdlib>
#include <atomic>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

// atomic variables used for metrics computation
atomic<long> total_requests_completed(0);
atomic<long> total_requests_made(0);
atomic<long long> total_latency_ns(0);

// type of workload and total run time
int g_workload_type; // 0=pre-populate db, 1=long-tail, 2=short-tail
int g_duration_sec;  // total run time in seconds

// vocab generation process
vector<string> popular_words = {
    "apple", "amazon", "google", "microsoft", "tcs", "reliance", "iitb",
    "facebook", "meta", "tesla", "nvidia", "intel", "oracle", "ibm",
    "flipkart", "uber", "airbnb", "zoom", "slack", "spotify", "twitter",
    "linkedin", "microsoftteams", "github", "docker", "kubernetes",
    "tensorflow", "pytorch", "scikit-learn", "opencv", "nlp", "reinforcement",
    "deepdream", "chatgpt", "stable-diffusion", "midjourney", "gpt4",
    "transformer", "attention", "backpropagation", "gradientdescent",
    "overfitting", "regularization", "dropout", "epoch", "batchnorm",
    "funnycat", "robotunicorn", "quantumbanana", "datawizard", "blah", "decs", "load", "testing"};

vector<string> random_words;
vector<string> gibberish_words;

// initialize the random words
void init_random_vocab(int n_words = 10000)
{
    for (int i = 0; i < n_words; i++)
    {
        random_words.push_back("word_" + to_string(i));
    }
}

// initialize gibberish words - just to make document length longer
void init_gibberish(int n_words = 5000)
{
    for (int i = 0; i < n_words; i++)
    {
        gibberish_words.push_back("gibberish_" + to_string(i));
    }
}

// function to generate documents, can be popular or un-popular
string generate_document(bool popular_heavy)
{
    string doc = "";
    int doc_size = 35; // average size of each word is 30-40

    for (int i = 0; i < doc_size; i++)
    {
        if (popular_heavy && (i == 0 || i == 1))
        {
            doc += popular_words[rand() % popular_words.size()] + " ";
        }
        else if (!popular_heavy && (i == 0 || i == 1))
        {
            doc += random_words[rand() % random_words.size()] + " ";
        }
        else
        {
            // fill other part with gibberish
            doc += gibberish_words[rand() % gibberish_words.size()] + " ";
        }
    }
    return doc;
}

// curl based functions
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// sending post request via CURL
bool send_post_request(const string &url, const string &json_body, string &resp_body, long timeout_ms = 3000)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    // curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms); // set the total request timeout, if no response from server in this much time then drop it.
    // curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000L); //set connection timeout if required

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

// sending get request via curl
bool send_get_request(const string &url, string &resp_body, long timeout_ms = 5000)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

// worker thread
void *client_work(void *arg_ptr)
{
    int thread_id = *(int *)arg_ptr;
    string server_base = "http://localhost:8080";
    auto start_time = chrono::steady_clock::now();

    while (true)
    {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - start_time).count() >= g_duration_sec)
            break;

        bool ok = false;
        auto t1 = chrono::steady_clock::now();

        if (g_workload_type == 0)
        {
            // this function is only being used for pre-populating database
            bool popular_heavy = (rand() % 10 < 1); // approx 1/10 of docs popular-heavy
            string doc = generate_document(popular_heavy);

            // cout << "String of document generated: " << doc << endl;

            json payload = {{"text", doc}};
            string resp_body;
            ok = send_post_request(server_base + "/documents", payload.dump(), resp_body);

            if (ok)
            {
                try
                {
                    auto j = json::parse(resp_body);
                    cout << "Thread " << thread_id
                         << " Inserted doc: " << j["document_id"]
                         << " Status: " << j["status"] << endl;
                }
                catch (...)
                {
                    cerr << "Thread " << thread_id << " Error parsing Insert response" << endl;
                }
            }
            else
            {
                cerr << "Thread " << thread_id << " Failed to insert document" << endl;
            }
        }
        else
        { // QUERY workloads
            string query_word = (g_workload_type == 1) ? random_words[rand() % random_words.size()] : popular_words[rand() % popular_words.size()];
            string url = server_base + "/search?query=" + query_word;

            string resp;
            ok = send_get_request(url, resp);

            if (ok)
            {
                try
                {
                    auto j = json::parse(resp);
                    size_t results_count = j["results"].size();
                    cout << "Thread " << thread_id << " Query: " << query_word
                         << " -> Results count: " << results_count << endl;
                    // cout << resp << endl;
                }
                catch (...)
                {
                    cerr << "Thread " << thread_id << " Error parsing JSON for query: " << endl;
                }
            }
        }

        auto t2 = chrono::steady_clock::now();
        total_requests_made++;

        if (ok)
        {
            long latency_ns = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();
            total_latency_ns += latency_ns;
            total_requests_completed++;
        }
    }

    return nullptr;
}

// main function
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        cout << "Usage: ./load_gen <threads> <duration_sec> <workload_type (0=insert for pre-population,1=long-tail,2=short-tail)>\n";
        return 0;
    }

    int num_threads = atoi(argv[1]);
    g_duration_sec = atoi(argv[2]);
    g_workload_type = atoi(argv[3]);

    srand(time(nullptr));
    init_random_vocab();
    init_gibberish();

    vector<pthread_t> threads(num_threads);
    vector<int> thread_ids(num_threads);

    for (int i = 0; i < num_threads; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, client_work, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], nullptr);

    double throughput = total_requests_completed.load() / static_cast<double>(g_duration_sec);
    double avg_response_time_ms = (total_latency_ns.load() / 1e6) /
                                  (total_requests_completed.load() > 0 ? total_requests_completed.load() : 1.0);

    cout << "\n========= RESULTS =========" << endl;
    cout << "Total requests made: " << total_requests_made.load() << endl;
    cout << "Total requests completed: " << total_requests_completed.load() << endl;
    cout << "Throughput: " << throughput << " req/sec" << endl;
    cout << "Avg response time: " << avg_response_time_ms << " ms" << endl;
    cout << "===========================" << endl;

    return 0;
}