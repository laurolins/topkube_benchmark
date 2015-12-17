#include "problem.hh"

#include <iostream>
#include <fstream>
#include <cassert>

#define xLOG_PRODUCTIONS

namespace problem {

    //----------------------------------------------------------------------------
    // Problem
    //----------------------------------------------------------------------------

    Rank* Problem::rank() { 
        auto r=new Rank(); 
        _ranks.push_back(std::unique_ptr<Rank>(r)); 
        return r;
    }

    //----------------------------------------------------------------------------
    // Rank
    //----------------------------------------------------------------------------

    void Rank::entry(const Entry& e) {
        _entries.push_back(e);
    }

    //----------------------------------------------------------------------------
    // Parser
    //----------------------------------------------------------------------------

    bool Parser::parse(Scanner &scanner) {
        // start new probltm
        _problem.reset(new Problem());
        _rank = nullptr;

        _scanner = &scanner;
        _scanner_error = false;
        _token_available = true; // flag as available before the first call to advance
        advance();

        this->P(0);

        if (token_available())
            _ok = false; // should have no more tokens on the scanner

        return _ok;
    }
    
    void Parser::swap_problems(std::unique_ptr<Problem> &ptr) {
        _problem.swap(ptr);
    }

    std::string Parser::tokstr() { 
        return std::string(_token.ts(), _token.te());
    }

    void Parser::P(int d) {

        if (!token_available()) { _ok=false; return; }

        if (_token.type() == TK_LBracket) { // E -> ident X

#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "P -> '[' R ']'" << std::endl;
#endif

            advance();

            R(d+1);

            if (!advance_if(TK_RBracket)) { _ok=false;  return; }

        }
        else {
            _ok = false;
        }
    }

    void Parser::R(int d) {

        if (!token_available()) { _ok=false; return; }

        if (_token.type() == TK_LBracket) { // E -> ident X

#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "R -> '[' L ']' RC" << std::endl;
#endif

            advance();

            {
                _rank = _problem->rank(); // new rank
            }

            L(d+1);

            {
                _rank = nullptr; // new rank
            }

            if (!advance_if(TK_RBracket)) { _ok=false;  return; }

            RC(d+1);

        }
        else {
            _ok = false;
        }

    }

    void Parser::RC(int d) {

        if (token_available()  && _token.type() == TK_Comma) { // E -> ident X

#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "RC -> ',' '[' L ']' RC" << std::endl;
#endif

            advance();

            if (!advance_if(TK_LBracket)) { _ok=false;  return; }

            {
                _rank = _problem->rank(); // new rank
            }

            L(d+1);

            {
                _rank = nullptr; // new rank
            }


            if (!advance_if(TK_RBracket)) { _ok=false;  return; }

            RC(d+1);

        } 
        else {
#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "RC -> empty" << std::endl;
#endif
        }

    }

    void Parser::L(int d) {

        if (!token_available()) { _ok=false; return; }

        if (_token.type() == TK_LBracket) { // E -> ident X

#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "L -> '[' E LC" << std::endl;
#endif
            advance();
            
            E(d+1);

            LC(d+1);

        }

    }

    void Parser::LC(int d) {

        if (token_available()  && _token.type() == TK_Comma) { // E -> ident X

#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "LC -> ',' '[' E LC" << std::endl;
#endif

            advance();

            if (!advance_if(TK_LBracket)) { _ok=false;  return; }

            E(d+1);

            LC(d+1);

        } 
        else {
#ifdef LOG_PRODUCTIONS
            std::cerr << std::string(4*d,'.') << "LC -> empty" << std::endl;
#endif
        }

    }

    void Parser::E(int d) { // list of triples

#ifdef LOG_PRODUCTIONS
        std::cerr << std::string(4*d,'.') << "E -> i ',' i ',' i ']'" << std::endl;
#endif

        int id, value, order;

        std::string buffer;

        buffer = tokstr();

        if (!advance_if(TK_Integer))  { _ok=false;  return; }

        id = std::stoull(buffer);
        
        if (!advance_if(TK_Comma))    { _ok=false;  return; }
        
        buffer = tokstr();

        if (!advance_if(TK_Integer))  { _ok=false;  return; }

        value = std::stoull(buffer);

        if (!advance_if(TK_Comma))    { _ok=false;  return; }
         
        buffer = tokstr();

        if (!advance_if(TK_Integer))  { _ok=false;  return; }

        order = std::stoull(buffer);
        
        if (!advance_if(TK_RBracket)) { _ok=false;  return; }

        if (!_rank) { _ok=false;  return; }

        _rank->entry({id, value, order});
    }

    void Parser::advance() {
        assert(_token_available && _scanner);
        auto tok = _scanner->next();
        if (tok) {
            _token = *tok;
            // std::cout << scanner::token_type_name(_token.type()) << std::endl;
        }
        else {
            if (_scanner->error())
                _scanner_error = true;
            _token_available = false;
        }
    }

    bool Parser::advance_if(TokenType tok) { 
        if (token_available() && _token.type() == tok) {
            advance();
            return true;
        }
        return false;
    }

}

