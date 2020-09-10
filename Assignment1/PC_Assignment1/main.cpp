//
//  main.cpp
//  PC1
//
//  Created by Varun Devgon on 10.04.20.
//  Copyright © 2020 Varun Devgon. All rights reserved.
//

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <numeric>
#include <atomic>

using namespace std;

struct generator_config
{
    generator_config(int max) : max(max) {}
    int max;
    
};

struct generator
{
    generator(const generator_config& cfg) : dist(0, cfg.max) {}
    int operator()()
    {
        return dist(engine);
    }
private:
    default_random_engine engine;
    uniform_int_distribution<int> dist;
};


struct histogram
{   public:
    histogram(int count, int mPerBucket, int variant) : data(count), data_atomic(count), m2(count/mPerBucket+1), m1(count) ,m(mPerBucket), variant(variant){ }
    void add(int i)
    {
    
        switch (variant) {
                
                case bucket_as_atomic:{
                    data[i] = ++data_atomic[i];
                    break;}
                
                case single_mutex:{
                    lock_guard<mutex> guard(mut);
                    ++data[i];
                    break;}
                
                case mutex_foreach_bucket:{
                    lock_guard<mutex> guard(m1[i]);
                    ++data[i];
                    break;}
                
                case mutex_perm_buckets:{
                    lock_guard<mutex> guard(m2[ceil(i/m)]);
                    ++data[i];
                    break;}
                
            default: ++data[i];
        }
        
    }
    void print()
    {
        for (size_t i = 0; i < data.size(); ++i) cout << i << ":" << data[i] << endl;
        cout << "total:" << accumulate(data.begin(), data.end(), 0) << endl;
    }
private:
    vector<int> data;       // histogram data with every element as bucket
    int m, variant;
    enum var{bucket_as_atomic=1, single_mutex=2, mutex_foreach_bucket=3, mutex_perm_buckets=4};     // Switch cases in add()
    vector<atomic<int>> data_atomic;    // atomic int for every histogram bucket
    mutex mut;          //single mutex for all histogram buckets
    vector<mutex> m1, m2;   // mutex vectors for providing mutexes for each/some histogram buckets
};


struct worker
{
    worker(int repeats_to_do, histogram& h, const generator_config& cfg) : repeats_to_do(repeats_to_do), h(h), cfg(cfg) {}
    void operator()()
    {
        generator gen(cfg);
        while (repeats_to_do--)
        {
            int next = gen(); // random number generated here
            h.add(next);
        }
        
    }
private:
    int repeats_to_do;
    histogram& h;
    const generator_config& cfg;
};

int main()
{
    int num_threads = std::thread::hardware_concurrency();
    cout << "Total concurrent threads supported  = " << num_threads << endl;
    int max = 10;
    int bucket_size = 4;
    int variant;
    cout << "Variant options : \nHistogram buckets as atomics ==> 1\nSingle mutex for the histogram ==> 2\nMutex for each bucket ==> 3\nMutex per M="<<bucket_size<<" buckets ==> 4\nNo synchronisation ==> Anything else\nEnter choice : ";
    cin >> variant;
    histogram h(max+1, bucket_size, variant);
    int repeats_to_do = 10000000;
    vector<thread> threads;
    generator_config cfg(max);
    auto t1 = chrono::high_resolution_clock::now();
for (int t = 1; t <= num_threads; ++t)
    {
    
        int from = (repeats_to_do*t / num_threads);
        int to = (repeats_to_do*(t+1) / num_threads);
        cout << "Jobs taken by thread " << t << " = " << to - from << endl;
        threads.push_back(thread(worker(to - from, h, cfg)));
    }
    for (auto& t : threads) t.join();
    auto t2 = chrono::high_resolution_clock::now();
    h.print();
    cout << "Time in seconds = "<<chrono::duration<double>(t2 - t1).count() << endl;
}

