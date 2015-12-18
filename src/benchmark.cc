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
    
    operator bool() const { return _present; }
    void reset() { _present = false; }
    T    _object;
    bool _present { false };
};

struct RunInfo {

    RunInfo& algorithm(const std::string& alg) { _algorithm.reset(alg); return *this; }
    
    RunInfo& k(RankSize k) { _k.reset(k); return *this; }
    
    RunInfo& time_seconds(float t) { _time_seconds.reset(t); return *this; }

    RunInfo& threshold(float t) { _threshold.reset(t); return *this; }

    const Maybe<std::string>& algorithm() const { return _algorithm; }
    const Maybe<RankSize>&    k() const { return _k; }
    const Maybe<float>   &    threshold() const { return _threshold; }
    const Maybe<float>   &    time_seconds() const { return _time_seconds; }
    const Maybe<RankSize>&    ta_k() const { return _ta_k; }
    const Maybe<Count>   &    ta_num_ranks() const { return _ta_num_ranks; }
    const Maybe<Count>   &    ta_largest_rank() const { return _ta_largest_rank; }
    const Maybe<Count>   &    ta_entries() const { return _ta_entries; }
    const Maybe<Count>   &    ta_total_access() const { return _ta_total_access; }
    const Maybe<Count>   &    ta_ordered_access_hit() const { return _ta_ordered_access_hit; }
    const Maybe<Count>   &    ta_ordered_access_miss() const { return _ta_ordered_access_miss; }
    const Maybe<Count>   &    ta_random_access_hit() const { return _ta_random_access_hit; }
    const Maybe<Count>   &    ta_random_access_miss() const { return _ta_random_access_miss; }
    const Maybe<RankSize>&    sweep_k() const { return _sweep_k; }
    const Maybe<Count>   &    sweep_largest_rank() const { return _sweep_largest_rank; }
    const Maybe<Count>   &    sweep_num_ranks() const { return _sweep_num_ranks; }
    const Maybe<Count>   &    sweep_entries() const { return _sweep_entries; }

    void reset() {
        _algorithm.reset();
        _k.reset();
        _threshold.reset();
        _time_seconds.reset();
        _ta_k.reset();
        _ta_num_ranks.reset();
        _ta_largest_rank.reset();
        _ta_entries.reset();
        _ta_total_access.reset();
        _ta_ordered_access_hit.reset();
        _ta_ordered_access_miss.reset();
        _ta_random_access_hit.reset();
        _ta_random_access_miss.reset();
        _sweep_k.reset();
        _sweep_largest_rank.reset();
        _sweep_num_ranks.reset();
        _sweep_entries.reset();
    }

    RunInfo& ta_info(const topk_algorithms::ThresholdAlgorithmStats& ta_stats) {
        _ta_k.reset(ta_stats.k);
        _ta_num_ranks.reset(ta_stats.num_ranks);
        _ta_largest_rank.reset(ta_stats.largest_rank);
        _ta_entries.reset(ta_stats.entries);
        _ta_total_access.reset(ta_stats.total_accesses());
        _ta_ordered_access_hit.reset(ta_stats.ordered_access_hit);
        _ta_ordered_access_miss.reset(ta_stats.ordered_access_miss);
        _ta_random_access_hit.reset(ta_stats.random_access_hit);
        _ta_random_access_miss.reset(ta_stats.random_access_miss);
        return *this;
    }

    RunInfo& sweep_info(const topk_algorithms::SweepAlgorithmStats& sweep_stats) {
        _sweep_k.reset(sweep_stats.k);
        _sweep_num_ranks.reset(sweep_stats.num_ranks);
        _sweep_largest_rank.reset(sweep_stats.largest_rank);
        _sweep_entries.reset(sweep_stats.entries);
        return *this;
    }

    Maybe<std::string> _algorithm;
    Maybe<RankSize>    _k;

    Maybe<float>    _threshold;
    Maybe<float>    _time_seconds;
    
    // ta info
    Maybe<RankSize> _ta_k;
    Maybe<Count>    _ta_num_ranks;
    Maybe<Count>    _ta_largest_rank;
    Maybe<Count>    _ta_entries;
    Maybe<Count>    _ta_total_access;
    Maybe<Count>    _ta_ordered_access_hit;
    Maybe<Count>    _ta_ordered_access_miss;
    Maybe<Count>    _ta_random_access_hit;
    Maybe<Count>    _ta_random_access_miss;
    
    // sweep info
    Maybe<RankSize> _sweep_k;
    Maybe<Count>    _sweep_largest_rank;
    Maybe<Count>    _sweep_num_ranks;
    Maybe<Count>    _sweep_entries;
    
    // algorithm|time|k|threshold|ta_k|ta_num_ranks|ta_largest_rank|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_k|sweep_num_ranks|sweep_largest_rank|sweep_entries
    
};


struct RunInfoHeader {};

std::ostream& operator<<(std::ostream& os, const RunInfo& run_info) {
    if (run_info.algorithm())              os << *run_info.algorithm();              os << "|";
    if (run_info.k())                      os << *run_info.k();                      os << "|";
    if (run_info.time_seconds())           os << *run_info.time_seconds();           os << "|";
    if (run_info.threshold())              os << *run_info.threshold();              os << "|";
    if (run_info.ta_k())                   os << *run_info.ta_k();                   os << "|";
    if (run_info.ta_num_ranks())           os << *run_info.ta_num_ranks();           os << "|";
    if (run_info.ta_largest_rank())        os << *run_info.ta_largest_rank();        os << "|";
    if (run_info.ta_total_access())        os << *run_info.ta_total_access();        os << "|";
    if (run_info.ta_ordered_access_hit())  os << *run_info.ta_ordered_access_hit();  os << "|";
    if (run_info.ta_ordered_access_miss()) os << *run_info.ta_ordered_access_miss(); os << "|";
    if (run_info.ta_random_access_hit())   os << *run_info.ta_random_access_hit();   os << "|";
    if (run_info.ta_random_access_miss())  os << *run_info.ta_random_access_miss();  os << "|";
    if (run_info.sweep_k())                os << *run_info.sweep_k();                os << "|";
    if (run_info.sweep_num_ranks())        os << *run_info.sweep_num_ranks();        os << "|";
    if (run_info.sweep_largest_rank())     os << *run_info.sweep_largest_rank();     os << "|";
    if (run_info.sweep_entries())          os << *run_info.sweep_entries();
    return os;
}

std::ostream& operator<<(std::ostream& os, const RunInfoHeader& run_info_header) {
    os << "algorithm|k|time|threshold|ta_k|ta_num_ranks|ta_largest_rank|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_k|sweep_num_ranks|sweep_largest_rank|sweep_entries";
    return os;
}

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

    ost << "problem|dataset|num_ranks|largest_rank|keys|entries|density|rank_sizes|" << RunInfoHeader() << std::endl;
    
    //
    // sweep, ta, hybrid = 0.05 0.10 0.15 0.20 0.25 0.30 0.35 0.40 0.45 0.50 0.55 0.60 0.65 0.70 0.75 0.80 0.85 0.90 0.95
    // k = 10 20 40 80 160 320
    //
    
    std::vector<RankSize> ks         { 5 , 10, 20, 40, 80, 160, 320 };
    std::vector<float>    thresholds { 0.0f,0.05f,0.10f,0.15f,0.20f,0.25f,0.30f,0.35f,0.40f,0.45f,0.50f,0.55f,0.60f,0.65f,0.70f,0.75f,0.80f,0.85f,0.90f,0.95,1.0f };
    
    // std::istream& ist = std::cin;
    std::string line;
    int line_no = 0;
    while (std::getline(ist, line, '\n')) {
        ++line_no;
        
//        if (line_no != 4)
//            continue;

        std::stringstream ss(line);
        std::string token;
        
        std::getline(ss, token, '|');   auto problem_id = std::stoi(token);
        std::getline(ss, token, '|');   auto dataset = token;
        std::getline(ss, token, '|');   auto num_ranks = std::stoi(token);
        std::getline(ss, token, '|');   auto largest_rank = std::stoi(token);
        std::getline(ss, token, '|');   auto keys = std::stoi(token);
        std::getline(ss, token, '|');   auto entries = std::stoi(token);
        std::getline(ss, token, '|');   auto density = std::stof(token);
        std::getline(ss, token, '|');   auto rank_sizes = token;
        std::getline(ss, token, '|');   auto spec = token;
        
        
        RunInfo run_info;
        
        Scanner scanner(&spec[0], &spec[spec.size()]);
        auto ok = parser.parse(scanner);

        if (ok) {
            // std::cout << "ok " << line_no << std::endl;

            std::unique_ptr<RankList> rank_list;
            parser.swap_rank_list(rank_list);
            // std::cout << "#ranks: " << rank_list->size() << std::endl;
            
            // sort rank list
            rank_list->tag_sorted();
            
            // assuming parser problem is not dirty
            ProblemRankList problem(*rank_list.get());
            
            
            for (auto t: thresholds) {
                
                for (auto k: ks) {
                    run_info.reset();
                    run_info.k(k);
                    run_info.threshold(t);
                    
                    
                    if (t == 0.0f) { // ta
                        run_info.algorithm("ta");
                        algorithm_watch.start();
                        auto result_ta = threshold_algorithm(&problem, k);
                        auto time = algorithm_watch.elapsed();
                        run_info.time_seconds(time);
                        run_info.ta_info(result_ta.stats);
                    }
                    else if (t == 1.0f) { // sweep
                        run_info.algorithm("sweep");
                        algorithm_watch.start();
                        auto result_sweep = sweep(&problem, k);
                        auto time = algorithm_watch.elapsed();
                        run_info.time_seconds(time);
                        run_info.sweep_info(result_sweep.stats);
                    }
                    else {
                        run_info.algorithm("hybrid");
                        algorithm_watch.start();
                        auto result_hybrid = hybrid_algorithm(&problem, k,t);
                        auto time = algorithm_watch.elapsed();
                        run_info.time_seconds(time);
                        run_info.sweep_info(result_hybrid.sweep_stats);
                        run_info.ta_info(result_hybrid.ta_stats);
                    }

                    std::cerr << "alg:" << *run_info.algorithm() << " k:"<< k << " threshold:" << t << " time:" << *run_info.time_seconds() << std::endl;

                    // output
                    ost << problem_id << "|" << dataset << "|" << num_ranks << "|" << largest_rank << "|" << keys << "|" << entries << "|" << density << "|" << rank_sizes << "|";
                    ost << run_info << std::endl;
                }
            }

            std::cerr << "finished processing problem " << line_no << " time: " << watch.elapsed() << std::endl;

        }
        else {
            std::cerr << "problem " << line_no << std::endl;
        }
    }
}