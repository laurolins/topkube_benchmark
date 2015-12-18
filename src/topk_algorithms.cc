#include "topk_algorithms.hh"

#include <sstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace topk_algorithms {

    //------------------------------------------------------------------------------
    // TopK
    //------------------------------------------------------------------------------

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

    ThresholdAlgorithmInstance::ThresholdAlgorithmInstance(Count m) {
        for (auto i=0;i<m;++i)
            _rms_list.rank(); // create m ranks
    }

    ThresholdAlgorithmInstance::ThresholdAlgorithmInstance(RankList&& p) {
        std::swap(_rms_list,p);
    }

    const Rank& ThresholdAlgorithmInstance::rms(Index i) const { return *_rms_list.rank(i); }

    Count ThresholdAlgorithmInstance::num_ranks() const { return static_cast<Count>(_rms_list.size()); }

    void ThresholdAlgorithmInstance::add(Index i, const KeyValue& kv) { assert(i < _rms_list.size()); _rms_list.rank(i)->add(kv); _sum+=kv.value(); }

    void ThresholdAlgorithmInstance::add_new_key(Index i, const KeyValue& kv) { assert(i < _rms_list.size()); _rms_list.rank(i)->add_new_key(kv); _sum+=kv.value(); }

    void ThresholdAlgorithmInstance::sort() {
        _rms_list.sort();
    }

    Count ThresholdAlgorithmInstance::num_summands() const { return _rms_list.num_entries(); }

    Count ThresholdAlgorithmInstance::largest_rank() const { return _rms_list.largest_rank_size(); }

    Count ThresholdAlgorithmInstance::smallest_rank() const { return _rms_list.smallest_rank_size(); }

    std::string ThresholdAlgorithmInstance::ranks_str() const {
        std::vector<Count> ranks;
        for (auto i=0;i<_rms_list.size();++i) { ranks.push_back(_rms_list.rank(i)->size()); }
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

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //------------------------------------------------------------------------------
    // sweep algorithm
    //------------------------------------------------------------------------------
    
    SweepAlgorithmResult sweep(const Problem *problem, RankSize k) {
        
        struct Queue {
            Queue(const Rank* rank): _rank(rank) {};
            bool empty() const { return _index >= _rank->size(); }
            bool operator<(const Queue& other) const { return head_key() > other.head_key(); }
            Key head_key() const {
                assert(!empty());
                return _rank->operator[](_index).key();
            }
            const Entry* next() {
                if (_index < _rank->size()) {
                    auto &result = _rank->operator[](_index);
                    ++_index;
                    return &result;
                }
                else {
                    return nullptr;
                }
            }
            const Rank* _rank;
            RankSize _index { 0 };
        };
        
        // create m queues
        std::vector<Queue> queues;
        auto m = problem->size();
        Count num_entries = 0;
        queues.reserve(m);
        for (auto i=0;i<m;++i) {
            queues.push_back(Queue(problem->rank(i)));
            if (queues.back().empty()) {
                queues.pop_back();
            }
            else {
                num_entries += queues.back()._rank->size();
            }
        };
        std::make_heap(queues.begin(), queues.end());
        
        SweepAlgorithmResult result;
        result.topk.k(k);
        result.stats.k           = k;
        result.stats.num_entries = num_entries;
        TopK& topk = result.topk;
        
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
        return result;
    }

    
    
    
    
    
    
    //------------------------------------------------------------------------------
    // threshold algorithm
    //------------------------------------------------------------------------------

    ThresholdAlgorithmResult threshold_algorithm(const Problem *problem, RankSize k) {

        //
        struct Queue {
            Queue(const Rank* rank): _rank(rank) {};
            bool empty() const { return _index >= _rank->size(); }
            Value head_value() const { return _index < _rank->size() ? _rank->top(_index).value() : 0; }
            Value value_of(Key key) const { return _rank->value_of(key); }
            const Entry* next() {
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
            const Rank* _rank;
            RankSize _index { 0 };
        };

        // create m queues
        std::vector<Queue> queues;
        Value              head_sum = 0;
        auto m = problem->size();
        queues.reserve(m);
        for (auto i=0;i<m;++i) {
            queues.push_back(Queue(problem->rank(i)));
            if (!queues.back().empty()) {
                head_sum += queues.back().head_value();
            }
            else {
                queues.pop_back();
            }
        };

        std::unordered_set<Key> processed_keys;


        ThresholdAlgorithmResult result;
        result.topk.k(k);
        auto &topk  = result.topk;
        auto &stats = result.stats;
        stats.k = k;

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
    // hybrid algorithm
    //------------------------------------------------------------------------------
    
    //
    // If we merge all ranks from "L", this rank will have a certain size "s"
    // satisfying:
    //
    //     max_rank_size(L) <= s <= sum_rank_size(L)
    //
    // Let (A) density, (B) density_upper_bound, and (C) density_lower_bound of a
    // rank list "L" be defined as follows:
    //
    // (A) The density of "L" is given by
    //
    //           num_entries(L) / s * m
    //
    // (B) The density_upper_bound of "L" is given by
    //
    //           num_entries(L) / max_rank_size(L) * m
    //
    // (C) The density_lower_bound of "L" is given by
    //
    //           num_entries(L) / sum_rank_size(L) * m
    //
    //  When running the hybrid algorithm, we will use density_upper_bound_threshold
    //  to transform "L" into another equivalent rank list "L'" such that its
    //  density_upper_bound satisfies:
    //
    //           density_upper_bound(L') >=  density_upper_bound_threshold
    //
    //
    
    HybridAlgorithmResult hybrid_algorithm(const Problem *problem, RankSize k, float density_upper_bound_threshold) {

        HybridAlgorithmResult result;
        
        //
        auto m = problem->size();

        if (m == 0)
            return result;
        
        // max density == 1.0f
        if (density_upper_bound_threshold > 1.0f)
            density_upper_bound_threshold = 1.0f;
        
        
        // sort instance by decreasing ranks
        std::vector<const Rank*> ranks;
        ranks.reserve(m);
        for (auto i=0;i<m;++i) { ranks.push_back(problem->rank(i)); }
        
        // sort in decreasing order of entries
        std::sort(ranks.begin(), ranks.end(), [](const Rank* r1, const Rank* r2) { return r1->size() >= r2->size(); });
        
        // cumulative summands
        Count l = 0;
        {
            Count cum = 0;
            Count rank_size_0 = ranks[0]->size();
            for (auto r: ranks) {
                cum += r->size();
                auto d = (float)cum / (rank_size_0 * (l+1));
                if (d < density_upper_bound_threshold) {
                    break;
                }
                ++l;
            }
        }

        //
        // use sweep algorithm
        //
        // merge [ ranks.begin() + l, ranks.end() )
        //

        if (l == m) { // full TA
            
            auto ta_algorithm_result = threshold_algorithm(problem, k);
            
            result.ta_stats = ta_algorithm_result.stats;
            
            std::swap(result.topk, ta_algorithm_result.topk);
            
        }
        else {

            ProblemRankVector sweep_problem(ranks,l,m);

            auto sweep_result = sweep(&sweep_problem, 0); // run a whole sweep on all ranks form l to m
            
            result.sweep_stats = sweep_result.stats;
            
            // replace rank at position l with a new rank based on r
            Rank sweep_rank(sweep_result.topk.entries());
            sweep_rank.sort();
            
            // replace everything
            ranks[l] = &sweep_rank;
            ranks.erase(ranks.begin()+l+1,ranks.end());
            
            ProblemRankVector ta_problem(ranks,0,ranks.size());
            auto ta_result = threshold_algorithm(&ta_problem,k);
            
            
            std::swap(result.topk, ta_result.topk);
            result.ta_stats = ta_result.stats;
            
        }
        
        // auto ppos = pos.base();
        return result;
        
        
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
        instance.sort();

        return instance;

    };


    std::ostream& operator<<(std::ostream& os, const ThresholdAlgorithmInstance& instance) {
        os << "[";
        for (auto i=0;i<instance.num_ranks();++i) {
            os << "{";
            auto &rms = instance.rms(i);
            bool first = true;
            for (auto &r: rms._entries) {
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

} // namespace topk_algorithms





//// numbers are reversed here
//auto pos = std::upper_bound(density_estimates.begin(),
//                            density_estimates.end(),
//                            density_upper_bound_threshold,
//                            [](float a, float b) { return a > b; });

//                                  upper bound -----> *
// d1 >= d2 >= ... >= di >= t == t == t == t >= ... >= dj >= ... >= end
//
//
//                    di > t  > dj
//           upper bound -----> *
// d1 >= d2 >= ... >= di   >=   dj >= ... >= end
//
//
// pos is the first density smaller than "t"
//

//
// l is at least 0
//
// proof: density_estimates starts with a 1.0f so pos > density_estimates.begin()
//
// merge all densities with
//

