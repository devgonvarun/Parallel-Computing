#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <numeric>
#include <ctime>
#include <omp.h>

using namespace std;

struct generator_config
{
    generator_config(int max) : max(max) {}
    int max;
};

struct generator
{
    generator(const generator_config& cfg) : dist(0, cfg.max){}
    int operator()()
    {
        engine.seed(chrono::system_clock::now().time_since_epoch().count());
        return dist(engine);
    }
private:
    minstd_rand engine;
    uniform_int_distribution<int> dist;
};

struct histogram
{

    histogram(int count,int index) : data(count, vector<int> (index)), index(index), max(count)
    {
    }
    void add(int i, int ind2)
    {
        ++data[i][ind2];
    }
    void print()
    {
        int sum = 0;
        for (int i = 0; i < data.size(); i++) {
            for (int j = 0; j < data[i].size(); j++){
                sum = sum + data[i][j];
                
        }
            cout<<i << ":" << sum<< endl;
            sum = 0;
        }

        cout <<endl<<"total:" << accumulate(data.cbegin(), data.cend(), 0, [](auto lhs, const auto& rhs) {
            return std::accumulate(rhs.cbegin(), rhs.cend(), lhs);
        })<<endl;
    }
private:
    int max;
    int index;
    vector<vector<int>> data;
};


struct worker
{
    worker(histogram &h, const generator_config& cfg, int index) : h(h), cfg(cfg), index(index){}
    void operator()()
    {
        generator gen(cfg);
        int next = gen();
        h.add(next,index);
    }
private:
    int index;
    histogram& h;
    const generator_config& cfg;
};

int main()
{
    int max = 10;
    int repeats_to_do = 10000000;
    histogram h(max+1,repeats_to_do);
    generator_config cfg(max);

    int num_threads = 16; // Change value here to change number of threads
    omp_set_num_threads(num_threads);
    
    auto t1 = omp_get_wtime();

//#pragma omp parallel for schedule(static)
//#pragma omp parallel for schedule(dynamic)
//#pragma omp parallel for schedule(guided)
//#pragma omp parallel for schedule(runtime)
#pragma omp parallel for schedule(auto)
    for(int t=0;t<repeats_to_do;t++){
        worker(h, cfg,t)();
            }

    auto t2 = omp_get_wtime();
    
    cout << endl <<"Time in seconds = "<<chrono::duration<double>(t2 - t1).count() << endl<< endl;
    h.print();
}
