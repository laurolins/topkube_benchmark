#pragma once 

//#include <vector>
//#include <thread>
//#include <iostream>
//#include <numeric>
//#include <iomanip>
//#include <thread>
//#include <algorithm>
//#include <functional>
//#include <fstream>
//#include <mutex>
//#include <memory>
//#include <set>
//#include <cmath>
//#include <numeric>
//#include <unordered_set>
//#include <unordered_map>
//#include <cassert>

#include <random>
#include <memory>

#include "rank.hh"

namespace topk_algorithms {

    using namespace rank;

    //------------------------------------------------------------------------------
    // TopK
    //------------------------------------------------------------------------------

    struct TopK {
    public:
        TopK() = default;
        TopK(RankSize k): _k(k) {}
        void insert(const KeyValue& x);
        bool operator==(const TopK& other) const;
        RankSize count_entries_above_or_equal(Value threshold) const;

        RankSize size() const { return static_cast<RankSize>(_entries.size()); }
        RankSize k() const { return _k; }
  
        void k(RankSize k) { _k = k; }
        
        const std::vector<KeyValue>& entries() const { return _entries; }

    public:
        RankSize _k { 0 };
        std::vector<KeyValue> _entries; //
        // TODO: measure this! inserting on vectors with
        // too many objects might be slower since elements
        // might need to be shuffled around
        //
    };

    //------------------------------------------------------------------------------
    // Threshold Algorithm Instance
    //------------------------------------------------------------------------------

    struct ThresholdAlgorithmInstance {
    public:
        ThresholdAlgorithmInstance(Count m);
        ThresholdAlgorithmInstance(RankList&& p);
        const Rank& rms(Index i) const;
        Count num_ranks() const;
        void add(Index i, const KeyValue& kv);
        void add_new_key(Index i, const KeyValue& kv);
        void sort();
        Count num_summands() const;
        Count largest_rank() const;
        Count smallest_rank() const;
        std::string ranks_str() const;
        Value sum() const;
    public:
        RankList _rms_list;
        Value _sum { 0 };
    };

    //------------------------------------------------------------------------------
    // Problem
    //------------------------------------------------------------------------------

    struct Problem {
        virtual Count size() const = 0;
        virtual const Rank* rank(Index i) const = 0;
    };
    
    //------------------------------------------------------------------------------
    // ProblemRankList
    //------------------------------------------------------------------------------
    
    struct ProblemRankList: public Problem {
    public:
        ProblemRankList(const RankList& rank_list): _rank_list(rank_list) {}
        Count size() const { return _rank_list.size(); }
        virtual const Rank* rank(Index i) const { return _rank_list.rank(i); }
    public:
        const RankList& _rank_list;
    };

    //------------------------------------------------------------------------------
    // ProblemRankVector
    //------------------------------------------------------------------------------
    
    struct ProblemRankVector: public Problem {
    public:
        ProblemRankVector(const std::vector<const Rank*>& rank_vec, Index begin, Index end): _rank_vec(rank_vec), _begin(begin), _end(end) {}
        Count size() const { return _end-_begin; }
        virtual const Rank* rank(Index i) const { return _rank_vec.at(_begin + i); }
    public:
        const std::vector<const Rank*>& _rank_vec;
        Index                           _begin;
        Index                           _end;
    };

    //------------------------------------------------------------------------------
    // threshold algorithm
    //------------------------------------------------------------------------------

    struct ThresholdAlgorithmStats {
        ThresholdAlgorithmStats() = default;
        Count total_accesses() const {
            return ordered_access_hit + ordered_access_miss +
            random_access_hit + random_access_miss; }
        Count ordered_access_hit  { 0 };
        Count ordered_access_miss { 0 };
        Count random_access_hit   { 0 };
        Count random_access_miss  { 0 };
        Count k                   { 0 };
        Count num_ranks           { 0 };
        Count largest_rank        { 0 };
        Count entries             { 0 };
    };

    //------------------------------------------------------------------------------
    // threshold algorithm
    //------------------------------------------------------------------------------
    
    struct SweepAlgorithmStats {
        SweepAlgorithmStats() = default;
        Count num_ranks    { 0 };
        Count largest_rank { 0 };
        Count entries      { 0 };
        Count k            { 0 };
    };

    //------------------------------------------------------------------------------
    // Sweep Algorithm Result
    //------------------------------------------------------------------------------
    
    struct SweepAlgorithmResult {
        SweepAlgorithmResult() {}
        TopK topk;
        SweepAlgorithmStats stats;
    };

    //------------------------------------------------------------------------------
    // Threshold Algorithm Result
    //------------------------------------------------------------------------------
    
    struct ThresholdAlgorithmResult {
        ThresholdAlgorithmResult() {}
        TopK topk;
        ThresholdAlgorithmStats stats;
    };
    
    //------------------------------------------------------------------------------
    // Hybrid Algorithm Result
    //------------------------------------------------------------------------------
    
    struct HybridAlgorithmResult {
        HybridAlgorithmResult() = default;
        TopK                    topk;
        float                   threshold { 0.0f };
        ThresholdAlgorithmStats ta_stats;
        SweepAlgorithmStats     sweep_stats;
    };
    
    //------------------------------------------------------------------------------
    // sweep algorithm
    //------------------------------------------------------------------------------
    
    SweepAlgorithmResult sweep(const Problem *problem, RankSize k);
    
    //------------------------------------------------------------------------------
    // hybrid threshold algorithm
    //------------------------------------------------------------------------------
    
    ThresholdAlgorithmResult threshold_algorithm(const Problem *problem, RankSize k);
    
    //------------------------------------------------------------------------------
    // hybrid algorithm
    //------------------------------------------------------------------------------
    
    HybridAlgorithmResult hybrid_algorithm(const Problem *problem,
                                           RankSize k,
                                           float threshold);

    //------------------------------------------------------------------------------
    // threshold algorithm instance model
    //------------------------------------------------------------------------------
    
    // use Zipf's law in both: weight of the sets and weight of the vocabulary terms
    // the frequency of k-th ranked set/term, its frequency is 1/k that of the 1 ranked set/term
    //
    // how many
    //

    std::default_random_engine& gen();

    ThresholdAlgorithmInstance ta_zipf_instance_sample(Count vocabulary_size, Count num_ranks, Count sample_size);

    std::ostream& operator<<(std::ostream& os, const ThresholdAlgorithmInstance& instance);

    std::ostream& operator<<(std::ostream& os, const TopK& topk);


}