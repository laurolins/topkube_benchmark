
#line 1 "scanner.rl"
#include "scanner.hh"

#include <iostream>
#include <sstream>

//
// ragel scanner state machine description
//


#line 14 "scanner.cc"
static const char _Scanner_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7
};

static const char _Scanner_key_offsets[] = {
	0, 0, 9, 12
};

static const char _Scanner_trans_keys[] = {
	32, 44, 48, 91, 93, 9, 13, 49, 
	57, 32, 9, 13, 48, 57, 0
};

static const char _Scanner_single_lengths[] = {
	0, 5, 1, 0
};

static const char _Scanner_range_lengths[] = {
	0, 2, 1, 1
};

static const char _Scanner_index_offsets[] = {
	0, 0, 8, 11
};

static const char _Scanner_trans_targs[] = {
	2, 1, 1, 1, 1, 2, 3, 0, 
	2, 2, 1, 3, 1, 1, 1, 0
};

static const char _Scanner_trans_actions[] = {
	0, 11, 5, 7, 9, 0, 0, 0, 
	0, 0, 15, 0, 13, 15, 13, 0
};

static const char _Scanner_to_state_actions[] = {
	0, 1, 0, 0
};

static const char _Scanner_from_state_actions[] = {
	0, 3, 0, 0
};

static const char _Scanner_eof_trans[] = {
	0, 0, 14, 15
};

static const int Scanner_start = 1;
static const int Scanner_first_final = 1;
static const int Scanner_error = 0;

static const int Scanner_en_main = 1;


#line 33 "scanner.rl"


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
        
#line 92 "scanner.cc"
	{
	cs = Scanner_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 53 "scanner.rl"
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
            
            
#line 118 "scanner.cc"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _Scanner_actions + _Scanner_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 1:
#line 1 "NONE"
	{ts = p;}
	break;
#line 139 "scanner.cc"
		}
	}

	_keys = _Scanner_trans_keys + _Scanner_key_offsets[cs];
	_trans = _Scanner_index_offsets[cs];

	_klen = _Scanner_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _Scanner_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
_eof_trans:
	cs = _Scanner_trans_targs[_trans];

	if ( _Scanner_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _Scanner_actions + _Scanner_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 2:
#line 24 "scanner.rl"
	{te = p+1;{ token( TK_Integer ); {p++; goto _out; } }}
	break;
	case 3:
#line 25 "scanner.rl"
	{te = p+1;{ token( TK_LBracket ); {p++; goto _out; }  }}
	break;
	case 4:
#line 26 "scanner.rl"
	{te = p+1;{ token( TK_RBracket ); {p++; goto _out; }  }}
	break;
	case 5:
#line 27 "scanner.rl"
	{te = p+1;{ token( TK_Comma  ); {p++; goto _out; }  }}
	break;
	case 6:
#line 24 "scanner.rl"
	{te = p;p--;{ token( TK_Integer ); {p++; goto _out; } }}
	break;
	case 7:
#line 29 "scanner.rl"
	{te = p;p--;}
	break;
#line 228 "scanner.cc"
		}
	}

_again:
	_acts = _Scanner_actions + _Scanner_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 0:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 241 "scanner.cc"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _Scanner_eof_trans[cs] > 0 ) {
		_trans = _Scanner_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 70 "scanner.rl"
            
            if (cs == Scanner_error) {
                _error = true;
                return nullptr;
            }
            
        } while (_token.type() == TK_None);
        
        return &_token;

    }
    
}

