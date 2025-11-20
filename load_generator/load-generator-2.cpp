// LOAD GENERATOR FOR PUT REQUESTS

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <curl/curl.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

// generate a random long document
string random_text(int len = 1000)
{
    static const char chars[] =
        "abcdefghijklmnopqrstuvwxyz ";
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, sizeof(chars) - 2);

    string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i)
    {
        s.push_back(chars[dist(rng)]);
    }
    return s;
}

struct ThreadStats
{
    long requests = 0;
    long long total_latency_ms = 0;
};

void create_worker(int id, const string &url,
                   atomic<bool> &running,
                   ThreadStats &stats)
{
    // initialization of curl
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        cerr << "Thread " << id << ": CURL init failed\n";
        return;
    }

    // setting up timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    // function to handle output body
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void *, size_t s, size_t n, void *)
                     {
                         return s * n;
                     });

    while (running.load())
    {
        string text = random_text();
        string payload = "{\"text\": \"" + text + "\"}";

        // setting url and payload
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        // sending request
        auto start = steady_clock::now();
        CURLcode res = curl_easy_perform(curl);
        auto end = steady_clock::now();

        // parsing response
        if (res == CURLE_OK)
        {
            stats.requests++;
            stats.total_latency_ms +=
                duration_cast<milliseconds>(end - start).count();
        }
    }

    curl_easy_cleanup(curl);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << "Usage: " << argv[0]
             << " <num_threads> <duration_seconds> <create_url>\n";
        return 1;
    }

    int num_threads = stoi(argv[1]);
    int duration = stoi(argv[2]);
    string url = argv[3];

    atomic<bool> running(true);
    vector<thread> threads;
    vector<ThreadStats> stats(num_threads);

    cout << "Starting test: "
         << num_threads << " threads for "
         << duration << " seconds\n";

    for (int i = 0; i < num_threads; i++)
        threads.emplace_back(create_worker, i, url,
                             ref(running), ref(stats[i]));

    // running untill duration amount of time
    this_thread::sleep_for(seconds(duration));
    running.store(false);

    for (auto &t : threads)
        t.join();

    long total = 0;
    long long total_latency = 0;

    for (auto &s : stats)
    {
        total += s.requests;
        total_latency += s.total_latency_ms;
    }

    double avg_latency =
        total ? (double)total_latency / total : 0;

    cout << "\n========== RESULTS ==========\n";
    cout << "Total requests: " << total << "\n";
    cout << "Throughput: " << (total / duration) << " req/sec\n";
    cout << "Avg latency: " << avg_latency << " ms\n";
    cout << "=============================\n";

    return 0;
}