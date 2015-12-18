#include <vector>
#include <thread>
#include <iostream>
#include <numeric>
#include <iomanip>
#include <thread>
#include <algorithm>
#include <functional>
#include <fstream>
#include <mutex>
#include <memory>
#include <set>
#include <cmath>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <random>

#include "tclap/CmdLine.h"



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


//------------------------------------------------------------------------------
// TopK
//------------------------------------------------------------------------------

struct TopK {
public:
    TopK(RankSize k=0): _k(k) {}
    void insert(const KeyValue& x);
    bool operator==(const TopK& other) const;
    RankSize count_entries_above_or_equal(Value threshold) const;

    RankSize size() const { return static_cast<RankSize>(_entries.size()); }
    RankSize k() const { return _k; }
public:
    RankSize _k { 0 };
    std::vector<KeyValue> _entries; //
    // TODO: measure this! inserting on vectors with
    // too many objects might be slower since elements
    // might need to be shuffled around
    //
};

// TopK Impl.

void TopK::insert(const KeyValue& x) {
    auto it = std::upper_bound(_entries.begin(),_entries.end(), x);
    // assuming keys are different
    _entries.insert(it, x);
    if (_k && _entries.size() > _k) {
        _entries.pop_back();
    }
}

bool TopK::operator==(const TopK& other) const {
    if (_entries.size() != other._entries.size())
        return false;
    for (auto it=_entries.begin(),it2=other._entries.begin();it!=_entries.end();++it,++it2) {
        if (*it != *it2)
            return false;
    }
    return true;
}

RankSize TopK::count_entries_above_or_equal(Value threshold) const {
    auto entries_above_threshold = size();
    for (auto it=_entries.rbegin();it!=_entries.rend();++it) {
        if (it->value() < threshold) {
            --entries_above_threshold;
        }
        else {
            break;
        }
    }
    return entries_above_threshold;
}

//------------------------------------------------------------------------------
// Threshold Algorithm Instance
//------------------------------------------------------------------------------

struct ThresholdAlgorithmInstance {
    ThresholdAlgorithmInstance(Count m): _rms_list(m) {}; // number of ranks
    const RankedMultiSet& rms(Index i) const { return _rms_list[i]; }
    Count num_ranks() const { return static_cast<Count>(_rms_list.size()); }
    void add(Index i, const KeyValue& kv) { assert(i < _rms_list.size()); _rms_list[i].add(kv); _sum+=kv.value(); }
    void add_new_key(Index i, const KeyValue& kv) { assert(i < _rms_list.size()); _rms_list[i].add_new_key(kv); _sum+=kv.value(); }
    void rank() { std::for_each(_rms_list.begin(),_rms_list.end(),[](RankedMultiSet& rms) { rms.rank(); }); }
    Count num_summands() const {
        Count sum = 0;
        for (auto& rms: _rms_list) {
            sum += rms.size();
        }
        return sum;
    }
    Count largest_rank() const {
        Count max = 0;
        for (auto& rms: _rms_list) {
            max = std::max(rms.size(),max);
        }
        return max;
    }

    Count smallest_rank() const {
        if (!_rms_list.size())
            return 0;
        auto min = _rms_list.at(0).size();
        for (auto& rms: _rms_list) {
            min = std::min(rms.size(),min);
        }
        return min;
    }

    std::string ranks_str() const {
        std::vector<Count> ranks;
        ranks.reserve(_rms_list.size());
        std::for_each(_rms_list.begin(), _rms_list.end(), [&ranks](const RankedMultiSet& rms) { ranks.push_back(rms.size()); });
        std::sort(ranks.begin(), ranks.end(), [](Count a, Count b) { return a > b; });
        std::stringstream ss;
        bool first = true;
        for (auto &r: ranks) {
            if (!first)
                ss << ",";
            ss << r;
            first = false;
        }
        return ss.str();
    }

    Value sum() const { return _sum; }
    
    std::vector<RankedMultiSet> _rms_list;
    Value _sum { 0 };
};


//------------------------------------------------------------------------------
// threshold algorithm
//------------------------------------------------------------------------------

struct ThresholdAlgorithmResult {
    ThresholdAlgorithmResult(RankSize k=0): topk(k) {}
    TopK topk;
    struct {
        Count total_accesses() const { return ordered_access_hit + ordered_access_miss + random_access_hit + random_access_miss; }
        Count ordered_access_hit  { 0 };
        Count ordered_access_miss { 0 };
        Count random_access_hit   { 0 };
        Count random_access_miss  { 0 };
    } stats;
};

ThresholdAlgorithmResult threshold_algorithm(const ThresholdAlgorithmInstance &instance, RankSize k) {

    //
    struct Queue {
        Queue(const RankedMultiSet* rank): _rank(rank) {};
        bool empty() const { return _index >= _rank->size(); }
        Value head_value() const { return _index < _rank->size() ? _rank->top(_index).value() : 0; }
        Value value_of(Key key) const { return _rank->value_of(key); }
        const Record* next() {
            if (_index < _rank->size()) {
                _head_value_drop = head_value();
                auto &result = _rank->top(_index);
                ++_index;
                _head_value_drop -= head_value();
                return &result;
            }
            else {
                return nullptr;
            }
        }
        Value head_value_drop() const { return _head_value_drop; }
        Value _head_value_drop { 0 };
        const RankedMultiSet* _rank;
        RankSize _index { 0 };
    };

    // create m queues
    std::vector<Queue> queues;
    Value              head_sum = 0;
    queues.reserve(instance.num_ranks());
    for (auto i=0;i<instance.num_ranks();++i) {
        queues.push_back(Queue(&instance.rms(i)));
        if (!queues.back().empty()) {
            head_sum += queues.back().head_value();
        }
        else {
            queues.pop_back();
        }
    };

    std::unordered_set<Key> processed_keys;


    ThresholdAlgorithmResult result(k);
    auto &topk  = result.topk;
    auto &stats = result.stats;

    
    std::size_t i = 0;
    while (i < queues.size()) {

        // for each queue generate next element
        auto &q = queues.at(i);

        auto top_record  = q.next();
        head_sum        -= q.head_value_drop();

        if ( top_record ) {
            if (!processed_keys.count(top_record->key())) {

                ++stats.ordered_access_hit;

                // if key has not beed processed
                auto record = KeyValue { top_record->key(), top_record->value() };

                // add all summands to record
                for (auto j=0;j<queues.size();++j) {
                    if (i != j) {
                        auto value = queues[j].value_of(record.key());
                        record.value_add(value);
                        if (value) {
                            ++stats.random_access_hit;
                        }
                        else {
                            ++stats.random_access_miss;
                        }
                    }
                }

                processed_keys.insert(record.key());

                topk.insert({record.key(), record.value()});

                if (k && topk.count_entries_above_or_equal(head_sum) >= k) {
                    // proven that we have a valid set of topk entries
                    break;
                }

                i = (i + 1) % queues.size();
            }
            else {
                ++stats.ordered_access_miss;
            }
        }
        else { // queue is emtpy
            std::swap(queues[i], queues.back());
            queues.pop_back();
            if (i >= queues.size()) {
                i = 0;
            }
        }
    }

    return result;
    
}












//------------------------------------------------------------------------------
// sweep algorithm
//------------------------------------------------------------------------------

TopK sweep(const ThresholdAlgorithmInstance &instance, RankSize k) {
    
    struct Queue {
        Queue(const RankedMultiSet* rank): _rank(rank) {};
        bool empty() const { return _index >= _rank->size(); }
        bool operator<(const Queue& other) const { return head_key() > other.head_key(); }
        Key head_key() const {
            assert(!empty());
            return _rank->operator[](_index).key();
        }
        const Record* next() {
            if (_index < _rank->size()) {
                auto &result = _rank->operator[](_index);
                ++_index;
                return &result;
            }
            else {
                return nullptr;
            }
        }
        const RankedMultiSet* _rank;
        RankSize _index { 0 };
    };
    
    // create m queues
    std::vector<Queue> queues;
    queues.reserve(instance.num_ranks());
    for (auto i=0;i<instance.num_ranks();++i) {
        queues.push_back(Queue(&instance.rms(i)));
        if (queues.back().empty()) {
            queues.pop_back();
        }
    };
    std::make_heap(queues.begin(), queues.end());
    
    TopK topk(k);
    
    bool first = true;
    KeyValue current_key_value;
    while (queues.size()) {
        // for each queue generate next element
        auto &q = queues.front();

        auto kv = q.next()->key_value();
        auto empty = q.empty();
        
        std::pop_heap(queues.begin(), queues.end());
        if (!empty)
            std::push_heap(queues.begin(), queues.end());
        else
            queues.pop_back();
        
        if (first) {
            current_key_value = kv;
            first = false;
        }
        else if (kv.key() != current_key_value.key()) {
            topk.insert(current_key_value);
            current_key_value = kv;
        }
        else {
            current_key_value.value_add(kv.value());
        }
    }
    if (!first) {
        topk.insert(current_key_value);
    }
    return topk;
}




//------------------------------------------------------------------------------
// threshold algorithm instance model
//------------------------------------------------------------------------------

// use Zipf's law in both: weight of the sets and weight of the vocabulary terms
// the frequency of k-th ranked set/term, its frequency is 1/k that of the 1 ranked set/term
//
// how many
//

std::default_random_engine& gen() {
    static std::default_random_engine generator(114727157);
    return generator;
}


ThresholdAlgorithmInstance ta_zipf_instance_sample(Count vocabulary_size, Count num_ranks, Count sample_size) {
    
    auto zipf_cum_probabilities = [](Count size) {
        std::vector<double> cum_rank_probability(size);
        {
            auto sum = 0.0;
            for (auto i=0;i<size;++i) {
                cum_rank_probability[i] = 1.0 / (1 + i);
                sum += cum_rank_probability[i];
            }
            auto cum = 0.0;
            for (auto i=0;i<size;++i) {
                cum += cum_rank_probability[i];
                cum_rank_probability[i] = cum / sum;
            }
        }
        return cum_rank_probability;
    };

    
    std::default_random_engine &generator = gen();
    
    auto sample = [&generator](const std::vector<double> &cum_probs, Count num_items) {
        std::vector<Index> sample(num_items);
        std::uniform_real_distribution<double> distribution(0.0,1.0);
        for (auto i=0;i<num_items;++i) {
            double uniform_sample = distribution(generator);
            sample[i] = std::distance(cum_probs.begin(),std::lower_bound(cum_probs.begin(),cum_probs.end(),uniform_sample));
        }
        return sample;
    };
    
    auto cum_ranks_probabilities = zipf_cum_probabilities(num_ranks);
    auto cum_terms_probabilities = zipf_cum_probabilities(vocabulary_size);
    
    auto rank_sample = sample(cum_ranks_probabilities,sample_size);
    auto term_sample = sample(cum_terms_probabilities,sample_size);

    struct Entry {
        Entry() = default;
        Entry(Index r, Index c, Value v):
            row(r), column(c), value(v)
        {
            key = std::to_string(row) + std::string(":") + std::to_string(column);
        }
        void add(Value v) { value += v; }
        std::string key;
        Index row { 0 };
        Index column { 0 };
        Value value { 0 };
    };

    std::unordered_map<std::string, Entry> entries;
    for (auto i=0;i<sample_size;++i) {
        Entry e { rank_sample[i], term_sample[i], 1 };
        auto it = entries.find(e.key);
        if (it == entries.end()) {
            entries[e.key] = e;
        }
        else {
            it->second.add(e.value);
        }
    }

    // sample
    ThresholdAlgorithmInstance instance(num_ranks);
    for (auto &it: entries) {
        auto &e = it.second;
        instance.add_new_key(e.row, KeyValue{e.column,e.value});
    }
    
    // order everything in the instance
    instance.rank();

    return instance;

};


std::ostream& operator<<(std::ostream& os, const ThresholdAlgorithmInstance& instance) {
    os << "[";
    for (auto i=0;i<instance.num_ranks();++i) {
        os << "{";
        auto &rms = instance.rms(i);
        bool first = true;
        for (auto &r: rms._records) {
            if (!first) {
                os << ",";
            }
            os << r.key() << ":" << r.value();
            first = false;
        }
        os << "}";
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TopK& topk) {
    bool first = true;
    os << "{";
    for (auto &kv: topk._entries) {
        if (!first)
            os << ",";
        os << kv.key() << ":" << kv.value();
        first = false;
    }
    os << "}";
    return os;
}


