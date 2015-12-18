#include "rank.hh"

#include <algorithm>

namespace rank {


//------------------------------------------------------------------------------
// Rank
//------------------------------------------------------------------------------

    void Rank::entry(const Entry& e) {
        _entries.push_back(e);
    }

    Rank::Rank(const std::vector<KeyValue>& key_values) {
        _dirty = true;
        _entries.reserve(key_values.size());
        for (auto &kv: key_values) { _entries.push_back(kv); }
    }

    Value Rank::value_of(Key key) const {
        auto r = find(key);
        return r ? r->value() : 0;
    }

    const Entry* Rank::find(Key key) const {
        auto it = std::lower_bound(_entries.begin(), _entries.end(), key, [](const Entry& r, Key key) { return r.key() < key; });
        return (it == _entries.end() || it->key() != key) ? nullptr : &(*it);
    }

    Entry* Rank::find(Key key) {
        auto it = std::lower_bound(_entries.begin(), _entries.end(), key, [](const Entry& r, Key key) { return r.key() < key; });
        return (it == _entries.end() || it->key() != key) ? nullptr : &(*it);
    }

    void Rank::add(const KeyValue& kv) {
        auto r = find(kv.key());
        if (!r) {
            _entries.push_back(Entry{kv});
            std::sort(_entries.begin(),_entries.end());
            _dirty = true;
        }
        else {
            r->key_value().value_add(kv.value());
            std::sort(_entries.begin(),_entries.end());
            _dirty = true;
        }
    }

    void Rank::add_new_key(const KeyValue& kv) {
        _entries.push_back(Entry{kv});
        _dirty = true;
    }

    void Rank::sort() {
    
        if (!dirty())
            return;
    
        std::sort(_entries.begin(),_entries.end());
    
        struct X {
            X(const Entry* record, Order index): record(record), index(index) {}
            bool operator<(const X& other) const { return record->key_value() < other.record->key_value(); }
            const Entry* record { nullptr };
            Order         index { 0 };
        };
    
        std::vector<X> aux;
        aux.reserve(_entries.size());
        Order i=0;
        std::for_each(_entries.begin(), _entries.end(), [&aux,&i](const Entry& r) {
                aux.push_back({&r,i++});
            });
        std::sort(aux.begin(), aux.end());
    
        i=0;
        std::for_each(_entries.begin(), _entries.end(), [&aux,&i](Entry& r) {
                r.order(aux[i++].index);
            });
    
        _dirty = false;
    }

    //----------------------------------------------------------------------------
    // RankList
    //----------------------------------------------------------------------------
    
    Rank* RankList::rank() {
        auto r=new Rank();
        _ranks.push_back(std::unique_ptr<Rank>(r));
        return r;
    }
    
    void RankList::sort() {
        for (auto &it: _ranks) {
            it->sort();
        }
    }
    
    Count RankList::num_entries() const {
        Count result = 0;
        for (auto &it: _ranks) {
            result += it->size();
        }
        return result;
    }

    Count RankList::largest_rank_size() const {
        Count result = 0;
        for (auto &it: _ranks) {
            result = std::max(result,it->size());
        }
        return result;
    }

    Count RankList::smallest_rank_size() const {
        Count result = 0;
        if (_ranks.size())
            result = _ranks[0]->size();
        for (auto &it: _ranks) {
            result = std::min(result,it->size());
        }
        return result;
    }


}
