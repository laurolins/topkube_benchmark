#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

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

    RunInfo& topk(const std::string& st) { _topk.reset(st); return *this; }

    RunInfo& k(RankSize k) { _k.reset(k); return *this; }

    RunInfo& topk_sum(Count topk_sum) { _topk_sum.reset(topk_sum); return *this; }

    RunInfo& time_seconds(float t) { _time_seconds.reset(t); return *this; }

    RunInfo& threshold(float t) { _threshold.reset(t); return *this; }


    const Maybe<std::string>& algorithm() const { return _algorithm; }
    const Maybe<std::string>& topk() const { return _topk; }
    const Maybe<Count>&       topk_sum() const { return _topk_sum; }
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
        _topk_sum.reset();
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
        _topk.reset();
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

    Maybe<float>        _threshold;
    Maybe<float>        _time_seconds;
    Maybe<Count>        _topk_sum;
    Maybe<std::string>  _topk;
    
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
    
    // algorithm|time|k|threshold|topk_sum|ta_k|ta_num_ranks|ta_largest_rank|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_k|sweep_num_ranks|sweep_largest_rank|sweep_entries
    
};


struct RunInfoHeader {};

std::ostream& operator<<(std::ostream& os, const RunInfo& run_info) {
    if (run_info.algorithm())              os << *run_info.algorithm();              os << "|";
    if (run_info.k())                      os << *run_info.k();                      os << "|";
    if (run_info.time_seconds())           os << *run_info.time_seconds();           os << "|";
    if (run_info.threshold())              os << *run_info.threshold();              os << "|";
    if (run_info.topk_sum())               os << *run_info.topk_sum();               os << "|";
    if (run_info.ta_k())                   os << *run_info.ta_k();                   os << "|";
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
    if (run_info.sweep_entries())          os << *run_info.sweep_entries();          os << "|";
    if (run_info.topk())                   os << *run_info.topk();
    return os;
}

std::ostream& operator<<(std::ostream& os, const RunInfoHeader& run_info_header) {
    os << "algorithm|k|time|threshold|topk_sum|ta_k|ta_num_ranks|ta_largest_rank|ta_total_access|ta_ordered_access_hit|ta_ordered_access_miss|ta_random_access_hit|ta_random_access_miss|sweep_k|sweep_num_ranks|sweep_largest_rank|sweep_entries|topk";
    return os;
}















struct Tasks {
    
    Tasks() {
        // make sure vector addresses are always the same
        _tasks.reserve(_capacity);
    }
    
    bool done() const {
        return !_more_tasks_coming && _tasks.empty();
    }
    
    void flag_no_more_tasks() {
        _more_tasks_coming = false;
    }
    
    void push_task(std::string& task_description) {
        std::lock_guard<std::mutex> guard(_mutex);
        _tasks.push_back(std::string());
        task_description.swap(_tasks.back());
    }
    
    bool pop_task(std::string& task_description) {
        bool result = false;
        if (_mutex.try_lock()) {
            if (!_tasks.empty()) {
                task_description.swap(_tasks.back());
                _tasks.pop_back();
                result = true;
            }
            _mutex.unlock();
        }
        return result;
    }
    
    bool full() {
        std::lock_guard<std::mutex> guard(_mutex);
        return _tasks.size() == _capacity;
    }

    int                      _capacity { 10 };
    std::mutex               _mutex;
    std::vector<std::string> _tasks;
    bool                     _more_tasks_coming { true };
    
};



struct MessageChannel {
    MessageChannel(std::ostream& os): _os(os) {}
    void send(const std::string &msg) {
        std::lock_guard<std::mutex> guard(_mutex);
        _os << msg;
    }
    std::ostream&            _os;
    std::mutex               _mutex;
};


void worker(int part, const std::string& base_name, Tasks& tasks, MessageChannel& msg_channel) {

    {
        std::stringstream ss;
        ss << "thread part " << part << " started" << std::endl;
        msg_channel.send(ss.str());
    }

    std::string part_filename = base_name + "_" + std::to_string(part);
    std::ofstream ost(part_filename.c_str());

    stopwatch::Stopwatch watch;
    watch.start();
    
    stopwatch::Stopwatch algorithm_watch;

    Parser  parser;
    
    bool done = false;
    while (true) {
        
        std::string task_description;
        while (true) {
            if (tasks.pop_task(task_description)) {
                break;
            }
            else {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                if (tasks.done()) {
                    done = true;
                    break;
                }
            }
        }
        
        if (done)
            break;
        
        // do the work!

        std::vector<RankSize> ks         { 5 , 10, 20, 40, 80, 160, 320 };
        std::vector<float>    thresholds { 0.0f, 0.025f, 0.05f,0.10f,0.15f,0.20f,0.25f,0.30f,0.35f,0.40f,0.45f,0.50f,0.55f,0.60f,0.65f,0.70f,0.75f,0.80f,0.85f,0.90f,0.95f,1.0f };
        
        //    std::vector<RankSize> ks         { 5 };
        //    std::vector<float>    thresholds { 0.0f,0.05f,0.10f,1.0f };
        
        auto register_topk = [](RunInfo& run_info, TopK& topk) {
            topk.sort();
            auto &entries = topk.entries();
            Count sum = 0;
            std::stringstream ss;
            bool first = true;
            for(auto it=entries.rbegin();it!=entries.rend();++it) {
                sum += it->value();
                if (!first) {
                    ss << ",";
                }
                ss << it->key() << ":" << it->value();
                first = false;
            }
            run_info.topk_sum(sum);
            run_info.topk(ss.str());
        };
        
        // std::istream& ist = std::cin;
        std::stringstream ss(task_description);
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
                        register_topk(run_info,result_ta.topk);
                    }
                    else if (t == 1.0f) { // sweep
                        run_info.algorithm("sweep");
                        algorithm_watch.start();
                        auto result_sweep = sweep(&problem, k);
                        auto time = algorithm_watch.elapsed();
                        run_info.time_seconds(time);
                        run_info.sweep_info(result_sweep.stats);
                        register_topk(run_info,result_sweep.topk);
                    }
                    else {
                        run_info.algorithm("hybrid");
                        algorithm_watch.start();
                        auto result_hybrid = hybrid_algorithm(&problem, k,t);
                        auto time = algorithm_watch.elapsed();
                        run_info.time_seconds(time);
                        run_info.sweep_info(result_hybrid.sweep_stats);
                        run_info.ta_info(result_hybrid.ta_stats);
                        register_topk(run_info,result_hybrid.topk);
                    }
                    
                    // std::cerr << "alg:" << *run_info.algorithm() << " k:"<< k << " threshold:" << t << " time:" << *run_info.time_seconds() << std::endl;
                    
                    // output
                    ost << problem_id << "|" << dataset << "|" << num_ranks << "|" << largest_rank << "|" << keys << "|" << entries << "|" << density << "|" << rank_sizes << "|";
                    ost << run_info << std::endl;
                }
            }
            
            {
                std::stringstream ss;
                ss << "finished processing problem " << problem_id << " time: " << watch.elapsed() << std::endl;
                msg_channel.send(ss.str());
            }
            
        }
        else {
            std::stringstream ss;
            ss << "problem at problem id: " << problem_id << " part " << part << std::endl;
            msg_channel.send(ss.str());
        }
    }
}






int main(int argc, char *argv[]) {

    if (argc != 4) {
        std::cout << "Usage: benchmark <input-problems-file> <output-topk-experiments-file> <threads>" << std::endl;
        return 0;
    }

    int parts = 1;
    
    try {
        parts = std::stoi(argv[3]);
        if (parts < 1) {
            std::cout << "Usage: benchmark <input-problems-file> <output-topk-experiments-file> <threads>" << std::endl;
            std::cout << "<threads> must be a positive number" << std::endl;
        }
    }
    catch(...) {
        std::cout << "Usage: benchmark <input-problems-file> <output-topk-experiments-file> <threads>" << std::endl;
        std::cout << "<threads> must be a positive number" << std::endl;
        return 0;
    }

    std::string base_name(argv[2]);
    
    std::istream *ist = &std::cin;
    std::ifstream ist_file;
    if (std::string(argv[1]).compare("-") != 0) {
        ist_file.open (argv[1],std::ifstream::in);
        ist = &ist_file;
    }

    // std::ofstream ost(argv[2]);

    //
    // assuming data is coming as dataset|spec
    //
    
    stopwatch::Stopwatch watch;
    watch.start();

    // stopwatch::Stopwatch algorithm_watch;
    // ost << "problem|dataset|num_ranks|largest_rank|keys|entries|density|rank_sizes|" << RunInfoHeader() << std::endl;

    Tasks          tasks;
    MessageChannel msg_channel(std::cerr);
    
    
    std::vector<std::unique_ptr<std::thread>> threads;
    for (int i=0;i<parts;++i) {
        threads.push_back(std::unique_ptr<std::thread>(new std::thread(worker, i, std::ref(base_name), std::ref(tasks), std::ref(msg_channel))));
    }
    
    std::string line;
    int line_no = 0;
    while (std::getline(*ist, line, '\n')) {
        ++line_no;
        if (line_no == 1) { // header hack
            continue;
        }
        tasks.push_task(line);
        while(true) {
            if (tasks.full()) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            else {
                break;
            }
        }
    }
    
    
    tasks.flag_no_more_tasks();
    
    int i = 0;
    for (auto &t: threads) {
        t->join();
        std::stringstream ss;
        ss << "joined thread " << i << std::endl;
        msg_channel.send(ss.str());
        ++i;
    }
    
}