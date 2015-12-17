#include "scanner.hh"

#include <iostream>
#include <sstream>

//
// ragel scanner state machine description
//

%%{

    machine Scanner;
    write data;

    fract_const   = digit* '.' digit+ | digit+ '.';
    exponent      = [eE] [+\-]? digit+;
    float_suffix  = [flFL];
    float_literal = ( fract_const exponent? float_suffix? | digit+ exponent float_suffix? );
    identifier    = ( [a-zA-Z_] [a-zA-Z0-9_]* );
    integer       = ( '0' | [1-9] [0-9]* );
    binop         = ( '+' | '-' | '/' | '*' | '<' | '>' | "<=" | ">=" );

    main := |*
    integer           => { token( TK_Integer ); fbreak; };
    '['               => { token( TK_LBracket ); fbreak;  };
    ']'               => { token( TK_RBracket ); fbreak;  };
    ','               => { token( TK_Comma  ); fbreak;  };
    
     space+;
    
     *|;

}%%

namespace scanner {

    const char* token_type_name(TokenType t) {
        static const char* names[] { 
        "TK_Error",
        "TK_Integer",
        "TK_Comma",
        "TK_LBracket",
        "TK_RBracket",
        "TK_None" 
        };
        return names[(int) t];
    }

    Scanner::Scanner(char* begin, char* end):
        p(begin), pe(end), eof(end)
    {
        %%write init;
    }
        
    void Scanner::token(TokenType tt) {
        _token = Token(tt, ts, te);
    }

    Token* Scanner::next() {
        
        _token = Token(TK_None, nullptr, nullptr);
        
        do {
            
            if (p == pe) {
                return nullptr;
            }
            
            %%write exec;
            
            if (cs == Scanner_error) {
                _error = true;
                return nullptr;
            }
            
        } while (_token.type() == TK_None);
        
        return &_token;

    }
    
}

