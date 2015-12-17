#include <memory>
#include <vector>

#include "scanner.hh"

namespace problem {
    
    using namespace scanner;

    struct Problem;
    struct Rank;
    struct Entry;

    //----------------------------------------------------------------------------
    // Problem
    //----------------------------------------------------------------------------

    struct Problem {
        Rank* rank(); // new rank
        std::vector<std::unique_ptr<Rank>> _ranks;
    };

    //----------------------------------------------------------------------------
    // Rank
    //----------------------------------------------------------------------------

    struct Rank {
        void entry(const Entry& e);
        std::vector<Entry> _entries;
    };

    //----------------------------------------------------------------------------
    // Entry
    //----------------------------------------------------------------------------

    struct Entry {
        Entry() = default;
        Entry(int key, int value, int order):
            _key(key), _value(value), _order(order) 
        {}
        int key() const   { return _key; }
        int value() const { return _value; }
        int order() const { return _order; }
        int _key ;
        int _value;
        int _order;
    };

    //----------------------------------------------------------------------------
    // Parser
    //
    // Grammar
    //
    // P -> '[' R ']'
    // R -> '[' L ']' RC
    // RC -> ',' '[' L ']' RC | empty
    // L -> '[' E C | empty 
    // LC -> ',' '[' E LC | empty
    // E -> int ',' int ',' int ']'
    //
    //----------------------------------------------------------------------------
    struct Parser {
    public:

        Parser() = default;

    public:

        bool parse(Scanner &scanner);

        const bool ok() const { return _ok; }

        void swap_problems(std::unique_ptr<Problem> &ptr);

    private:

        std::string tokstr();

        void P(int d); // problem
        void R(int d); // rank
        void RC(int d); // rank
        void L(int d); // list of triples
        void E(int d); // list of triples
        void LC(int d); // list continuation

        bool token_available() const { return _token_available; }

        void advance();

        bool advance_if(TokenType tok);

    public:

        std::unique_ptr<Problem> _problem;
        Rank* _rank { nullptr };

        Scanner *_scanner { nullptr };
        Token    _token; // current token
        bool     _token_available { false };
        bool     _scanner_error   { false };

        bool     _ok { true };
    };

}