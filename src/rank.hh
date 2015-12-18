#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <cassert>

namespace rank {

    //------------------------------------------------------------------------------
    // forward decl.
    //------------------------------------------------------------------------------

    using Key      = std::uint64_t;
    using Value    = std::uint64_t;
    using Order    = std::uint64_t;
    using RankSize = std::uint64_t;

    using Index    = std::uint64_t;
    using Count    = std::uint64_t;

    //------------------------------------------------------------------------------
    // KeyValue
    //------------------------------------------------------------------------------

    struct KeyValue {
    public:
        KeyValue() = default;
        KeyValue(Key key, Value value): _key(key), _value(value) {}
    
        // natural order for KeyValue is the greater the value the "smallest the KeyValue"
        bool operator<(const KeyValue &other) const { return _value > other._value || (_value == other._value && _key < other._key); }
        bool operator!=(const KeyValue &other) const { return _value != other._value || _key != other._key; }
        Key key() const { return _key; }
        Value value() const { return _value; }
        void value(Value v) { _value = v; }
        void value_add(Value v) { _value += v; }
    public:
        Key   _key   { 0 };
        Value _value { 0 };
    };


    //------------------------------------------------------------------------------
    // Entry
    //------------------------------------------------------------------------------

    struct Entry {
    public:

        Entry() = default;

        Entry(KeyValue kv): _key_value(kv) {}
        
        Entry(Key k, Value v, Order o): _key_value(k,v), _order(o) {}

        // natural order for a record is the key
        bool operator<(const Entry& other) const { return key() < other.key(); }

        KeyValue& key_value() { return _key_value; }

        const KeyValue& key_value()   const { return _key_value; }

        Key   key()   const { return _key_value.key(); }

        Value value() const { return _key_value.value(); }

        Key   order() const { return _order; }

        void  order(Order o) { _order = o; }

    public:
        KeyValue _key_value;
        Order    _order  { 0 };
    };

    //------------------------------------------------------------------------------
    // Rank
    //------------------------------------------------------------------------------

    struct Rank {
    public:
        Rank() = default;

        Rank(const std::vector<KeyValue>& key_values);

        Value value_of(Key key) const;

        Entry* find(Key key);

        void entry(const Entry& e);

        const Entry* find(Key key) const;

        void add(const KeyValue& kv) ;

        void add_new_key(const KeyValue& kv) ;

        Entry& operator[](RankSize index) { return _entries[index]; }

        const Entry& operator[](RankSize index) const  { return _entries[index]; }

        RankSize size() const { return static_cast<RankSize>(_entries.size()); }

        const Entry& top(RankSize index) const { assert(!dirty()); return _entries[_entries[index].order()]; }

        bool dirty() const { return _dirty; }

        // fill in the order field in the records
        // it should point to the correctly ranked record
        void sort();
    
    public:

        bool                _dirty { true };

        std::vector<Entry> _entries;

    };

    //----------------------------------------------------------------------------
    // RankList
    //----------------------------------------------------------------------------

    struct RankList {
        
        Rank* rank(Index i) { return _ranks.at(i).get(); } // new rank
        
        const Rank* rank(Index i) const { return _ranks.at(i).get(); } // new rank
        
        Count num_entries() const;

        Count largest_rank_size() const;

        Count smallest_rank_size() const;

        Rank* rank(); // new rank
        
        void sort();
        
        std::size_t size() const { return _ranks.size(); }
        
        std::vector<std::unique_ptr<Rank>> _ranks;
    };

}