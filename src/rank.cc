#include "rank.hh"

namespace rank {


    //----------------------------------------------------------------------------
    // RankList
    //----------------------------------------------------------------------------

    Rank* RankList::rank() { 
        auto r=new Rank(); 
        _ranks.push_back(std::unique_ptr<Rank>(r)); 
        return r;
    }

//------------------------------------------------------------------------------
// Rank
//------------------------------------------------------------------------------

    void Rank::entry(const Entry& e) {
        _entries.push_back(e);
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

    void Rank::rank() {
    
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

}