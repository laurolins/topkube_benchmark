#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "rank.hh"

#include "rank_parser.hh"

#include "topk_algorithms.hh"

using namespace scanner;
using namespace rank;
using namespace rank::parser;
using namespace topk_algorithms;


 //------------------------------------------------------------------------------
 // stopwatch
 //------------------------------------------------------------------------------

 namespace stopwatch {

     using  Seconds  = double; // seconds duration

     struct Stopwatch {
         using Timestamp = double;
         void start();
         Seconds elapsed() const;
         Timestamp _timestamp { 0 };
     };

     void timeit(const std::string& elapsed_msg="",
                 const std::string& msg="",
                 Stopwatch* stopwatch = nullptr);

 }

 namespace stopwatch {

     static Stopwatch stopwatch__;

     //
     // Stopwatch
     //

     void Stopwatch::start() {
         _timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
     }

     Seconds Stopwatch::elapsed() const {
         auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
         return (t - _timestamp)/1.0e9;
     }

     // timeit
     void timeit(const std::string& elapsed_msg,
                 const std::string& msg,
                 Stopwatch* stopwatch) {
         if (!stopwatch)
             stopwatch = &stopwatch__;

         if (msg.size()) {
             std::cerr << msg << std::endl;
         }
         if (elapsed_msg.size()) {
             std::cerr << elapsed_msg
             << " elapsed time: "
             << stopwatch->elapsed()
             << "s."
             << std::endl;
         }
         else {
             stopwatch->start();
         }
     }
 }


//
// (A)
// problem file format:
//
// problem|dataset|num_ranks|largest_rank|keys|entries|density|rank_sizes|spec
//

//
// (B)
// output
//
//

template <typename T>
struct Maybe {
    Maybe() = default;
    Maybe(const T& obj): _object(obj), _present(true) {}
    void reset(const T& obj) { _object = obj; _present = true; }
    const T& object() const { return _object; }
    const T& operator*() const { return _object; }
    void reset() { _present = false; }
    T    _object;
    bool _present { false };
};



struct Result {
    
    
    
    std::string  _algorithm;
    float        _time;
    RankSize     _k;

    Maybe<float>    _hybrid_threshold;
    
    // ta info
    Maybe<Count>    _ta_num_ranks;
    Maybe<RankSize> _ta_k;
    Maybe<Count>    _ta_keys;
    Maybe<Count>    _ta_total_access;
    Maybe<Count>    _ta_ordered_access_hit;
    Maybe<Count>    _ta_ordered_access_miss;
    Maybe<Count>    _ta_random_access_hit;
    Maybe<Count>    _ta_random_access_miss;
    
    // sweep info
    Maybe<Count>    _sweep_num_ranks;
    Maybe<RankSize> _sweep_k;
    Maybe<Count>    _ta_keys;
    Maybe<Count>    _ta_total_access;
    Maybe<Count>    _ta_ordered_access_hit;
    Maybe<Count>    _ta_ordered_access_miss;
    Maybe<Count>    _ta_random_access_hit;
    Maybe<Count>    _ta_random_access_miss;
    
    
    algorithm|time|k|hybrid_threshold|ta_num_ranks|ta_keys|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_num_ranks|sweep_keys|sweep_entries
    
    
};


int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cout << "Usage: benchmark <input-problems-file> <output-topk-experiments-file>" << std::endl;
        return 0;
    }
    Parser  parser;

    std::ifstream ist(argv[1]);
    std::ofstream ost(argv[2]);
    
    //
    // assuming data is coming as dataset|spec
    //
    
    stopwatch::Stopwatch watch;
    watch.start();

    stopwatch::Stopwatch algorithm_watch;

    std::cout << "problem|dataset|num_ranks|keys|entries|density|rank_sizes|algorithm|time|k|hybrid_threshold|ta_num_ranks|ta_keys|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_num_ranks|sweep_keys|sweep_entries" << std::endl;
    
    
    //
    // sweep, ta, hybrid = 0.05 0.10 0.15 0.20 0.25 0.30 0.35 0.40 0.45 0.50 0.55 0.60 0.65 0.70 0.75 0.80 0.85 0.90 0.95
    // k = 10 20 40 80 160 320
    //
    
    std::vector<RankSize> ks         {   5, 10, 20, 40, 80, 160, 320 };
    std::vector<float>    thresholds { 0.0f,0.05f,0.10f,0.15f,0.20f,0.25f,0.30f,0.35f,0.40f,0.45f,0.50f,0.55f,0.60f,0.65f,0.70f,0.75f,0.80f,0.85f,0.90f,0.95,1.0f };
    
    // std::istream& ist = std::cin;
    std::string line;
    int line_no = 0;
    while (std::getline(ist, line, '\n')) {
        ++line_no;

        std::stringstream ss(line);
        std::string token;
        
        std::getline(ss, token, '|');   auto problem = std::stoi(token);
        std::getline(ss, token, '|');   auto dataset = token;
        std::getline(ss, token, '|');   auto num_ranks = std::stoi(token);
        std::getline(ss, token, '|');   auto largest_rank = std::stoi(token);
        std::getline(ss, token, '|');   auto keys = std::stoi(token);
        std::getline(ss, token, '|');   auto entries = std::stoi(token);
        std::getline(ss, token, '|');   auto density = std::stof(token);
        std::getline(ss, token, '|');   auto rank_sizes = token;
        std::getline(ss, token, '|');   auto spec = token;
        
        
        Scanner scanner(&spec[0], &spec[spec.size()]);
        auto ok = parser.parse(scanner);

        if (ok) {
            // std::cout << "ok " << line_no << std::endl;

            std::unique_ptr<RankList> rank_list;
            parser.swap_rank_list(rank_list);
            // std::cout << "#ranks: " << rank_list->size() << std::endl;
            
            // sort rank list
            // rank_list->sort();
            
            // assuming parser problem is not dirty
            ProblemRankList problem(*rank_list.get());
            
            
            for (auto t: thresholds) {
                
                for (auto k: ks) {

                    if (t == 0.0f) { // ta
                        algorithm_watch.start();
                        auto result_ta = threshold_algorithm(&problem, k);
                        auto time = algorithm_watch.elapsed();


                    }
                    else if (t == 1.0f) { // sweep
                        algorithm_watch.start();
                        auto result_sweep = sweep(&problem, k);
                        auto time = algorithm_watch.elapsed();
                        
                        

                        // problem|dataset|num_ranks|keys|entries|density|algorithm|time|k|hybrid_threshold|ta_num_ranks|ta_keys|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_num_ranks|sweep_keys|sweep_entries

                        
                    }
                    else {
                        algorithm_watch.start();
                        auto result_hybrid = hybrid_algorithm(&problem, k,t);
                        auto time = algorithm_watch.elapsed();

                    }
                    
                }
                
            }
            
            
            
            
            
            auto result_sweep     = sweep(&problem, 0);
            // std::cout << "result_sweep:         " << result_sweep.topk << std::endl;
            
            
            auto num_ranks = rank_list->size();
            auto keys      = result_sweep.topk.size();
            auto entries   = rank_list->num_entries();
            auto density   = (float) entries / (keys * num_ranks);
            
            ost << line_no << '|' << dataset << '|' << num_ranks << '|' << largest_rank  << '|' << keys << '|' << entries << '|' << density << '|' << rank_sizes << '|' << std::string(line.begin()+split_pos+1,line.end()) << std::endl;

            std::cerr << "finished processing problem " << line_no << " time: " << watch.elapsed() << std::endl;

        }
        else {
            std::cerr << "problem " << line_no << std::endl;
        }
    }
}



// auto result_ta        = threshold_algorithm(&problem, 10);
// auto result_hybrid025 = hybrid_algorithm(&problem, 10, 0.25);
// std::cout << "result_ta:            " << result_ta.topk    << std::endl;
// std::cout << "result_hybrid_025:    " << result_hybrid025.topk    << std::endl;



// 
// //------------------------------------------------------------------------------
// // stopwatch
// //------------------------------------------------------------------------------
// 
// namespace stopwatch {
//     
//     using  Seconds  = double; // seconds duration
//     
//     struct Stopwatch {
//         using Timestamp = double;
//         void start();
//         Seconds elapsed() const;
//         Timestamp _timestamp { 0 };
//     };
//     
//     void timeit(const std::string& elapsed_msg="",
//                 const std::string& msg="",
//                 Stopwatch* stopwatch = nullptr);
//     
// }
// 
// namespace stopwatch {
//     
//     static Stopwatch stopwatch__;
//     
//     //
//     // Stopwatch
//     //
//     
//     void Stopwatch::start() {
//         _timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
//     }
//     
//     Seconds Stopwatch::elapsed() const {
//         auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
//         return (t - _timestamp)/1.0e9;
//     }
//     
//     // timeit
//     void timeit(const std::string& elapsed_msg,
//                 const std::string& msg,
//                 Stopwatch* stopwatch) {
//         if (!stopwatch)
//             stopwatch = &stopwatch__;
//         
//         if (msg.size()) {
//             std::cerr << msg << std::endl;
//         }
//         if (elapsed_msg.size()) {
//             std::cerr << elapsed_msg
//             << " elapsed time: "
//             << stopwatch->elapsed()
//             << "s."
//             << std::endl;
//         }
//         else {
//             stopwatch->start();
//         }
//     }
// }
// 
// 
// 


// //------------------------------------------------------------------------------
// // main
// //------------------------------------------------------------------------------
// 
// int main() {
// 
//     
//     // auto instance = ta_zipf_instance_sample(10,3,);
//     // std::cout << instance << std::endl;
//     // auto result = threshold_algorithm ( instance, 4);
//     // std::cout << result.topk << std::endl;
//     // std::cout << result.stats.total_accesses() << std::endl;
//     
//     // std::cout << instance << std::endl;
//     // std::cout << result.topk << std::endl;
//     // std::cout << result.stats.total_accesses() << std::endl;
//     
//     // parameters:
//     //
//     // n == number of keys
//     // s == number of samples (sum of all values)
//     // m == number of ranks
//     // r == repetitions
//     //
//     
//     std::vector<Count>  nn { 10, 100, 1000, 10000, 100000, 1000000 };
//     std::vector<Count>  ss { 1000,2000,4000,8000,16000,32000,64000,128000,256000,512000,1024000 };
//     // std::vector<Count>  mm { 1,2,4,8,16,32,64,128,256,512,1024 };
//     std::vector<Count>  mm { 2,4,8,16,32,64,128,256,512};
//     // std::iota(mm.begin(),mm.end(),1);
//     std::vector<Count>  kk { 2,4,8,16,32,64,128,256,512 };
//     int r = 1;
// 
//     // std::ofstream ostream("/Users/llins/projects/allocator/benchmarks/ta_zipf_m.psv");
//     std::ostream &os = std::cout;
//     os << "id|n|m|sample_size|k|uniquekeys|l_1|l_m|l_sum|accesses|ordered_access_hit|ordered_access_miss|random_access_hit|random_access_miss|l_all" << std::endl;
//     auto id = 0;
//     for (auto n: nn)
//         for (auto s: ss)
//             for (auto m: mm)
//                 for (auto k: kk)
//                     for (auto i=0;i<r;++i) {
//                         
//                         ++id;
//                         
//                         stopwatch::timeit();
//                     
//                         auto instance   = ta_zipf_instance_sample(n,m,s);
//                         auto result     = threshold_algorithm ( instance, k);
//                         auto sweep_topk = sweep(instance, 0);
//                         
//                         os
//                         << id       << "|"
//                         << n        << "|"
//                         << m        << "|"
//                         << s        << "|"
//                         << k        << "|"
//                         << sweep_topk.size()                     << "|"
//                         << instance.smallest_rank()              << "|"
//                         << instance.largest_rank()               << "|"
//                         << instance.num_summands()               << "|"
//                         << result.stats.total_accesses()         << "|"
//                         << result.stats.ordered_access_hit       << "|"
//                         << result.stats.ordered_access_miss      << "|"
//                         << result.stats.random_access_hit        << "|"
//                         << result.stats.random_access_miss       << "|"
//                         << instance.ranks_str()                  << std::endl;
// 
//                         stopwatch::timeit(std::to_string(id) + " solved");
//                     
//                     }
//     
//     
// //
// //    
// //    RankedMultiSet rms;
// //    rms.add(KeyValue{3,5});
// //    rms.add(KeyValue{2,19});
// //    rms.add(KeyValue{1,8});
// //    rms.rank(); // consolidate ranking
// //
// //    for (auto i=0;i<rms.size();++i) {
// //        auto &r = rms.top(i);
// //        std::cout << r.key() << " -> " << r.value() << std::endl;
// //    }
// //
// //
// //    
// //    auto result = threshold_algorithm ( { &rms, &rms }, 2);
// //
// //    std::cout << "total accesses: " << result.stats.total_accesses() << std::endl;
// //
// //    for (auto i=0;i<result.topk.size();++i) {
// //        auto &r = result.topk._entries[i];
// //        std::cout << r.key() << " -> " << r.value() << std::endl;
// //    }
// 
// }
// 
