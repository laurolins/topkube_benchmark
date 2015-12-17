#pragma once

namespace scanner {

    enum TokenType {
        TK_Error    = 0,
        TK_Integer  = 1,
        TK_Comma    = 2,
        TK_LBracket = 3,
        TK_RBracket = 4,
        TK_None     = 5
    };

    const char* token_type_name(TokenType t);

    struct Token {
        Token() = default;
        Token(TokenType type, const char* ts, const char* te): _type(type), _ts(ts), _te(te) {}
        TokenType type() const { return _type; }
        const char* ts() const { return _ts; }
        const char* te() const { return _te; }
        TokenType _type { TK_Error };
        const char *_ts;
        const char *_te;
    };

    struct Scanner {

        Scanner(char* begin, char* end);

        void token(TokenType tt);

        Token* next();

        bool error() const { return _error; }

        bool _error { false };

        Token _token;
        int   line = 1, col = 1;
        char *p, *pe, *eof; // token start, token end
        char *ts, *te; // token start, token end
        int   act, have = 0;
        int   cs;
    };

}
