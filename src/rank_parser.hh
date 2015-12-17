#include <memory>
#include <vector>

#include "scanner.hh"
#include "rank.hh"

//-------------
// Rank Parser
//-------------

namespace rank {

    namespace parser {
    
    using namespace scanner;

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

        void swap_problems(std::unique_ptr<RankList> &ptr);

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

        std::unique_ptr<RankList> _problem;
        Rank* _rank { nullptr };

        Scanner *_scanner { nullptr };
        Token    _token; // current token
        bool     _token_available { false };
        bool     _scanner_error   { false };

        bool     _ok { true };
    };

    } // parser

} // rank