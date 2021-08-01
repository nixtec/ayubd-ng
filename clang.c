
#line 1 "clang.rl"
/*
 * A mini C-like language scanner.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#line 91 "clang.rl"



#line 17 "clang.c"
static const char _clang_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 1, 
	16, 1, 17, 2, 0, 8, 2, 0, 
	9
};

static const char _clang_key_offsets[] = {
	0, 0, 3, 4, 7, 8, 9, 11, 
	17, 19, 22, 42, 44, 48, 50, 53, 
	59, 66
};

static const char _clang_trans_keys[] = {
	10, 34, 92, 10, 10, 39, 92, 10, 
	10, 48, 57, 48, 57, 65, 70, 97, 
	102, 10, 42, 10, 42, 47, 10, 34, 
	39, 47, 48, 95, 33, 46, 49, 57, 
	58, 64, 65, 90, 91, 96, 97, 122, 
	123, 126, 42, 47, 46, 120, 48, 57, 
	48, 57, 46, 48, 57, 48, 57, 65, 
	70, 97, 102, 95, 48, 57, 65, 90, 
	97, 122, 0
};

static const char _clang_single_lengths[] = {
	0, 3, 1, 3, 1, 1, 0, 0, 
	2, 3, 6, 2, 2, 0, 1, 0, 
	1, 0
};

static const char _clang_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 1, 3, 
	0, 0, 7, 0, 1, 1, 1, 3, 
	3, 0
};

static const char _clang_index_offsets[] = {
	0, 0, 4, 6, 10, 12, 14, 16, 
	20, 23, 27, 41, 44, 48, 50, 53, 
	57, 62
};

static const char _clang_trans_targs[] = {
	1, 10, 2, 1, 1, 1, 3, 10, 
	4, 3, 3, 3, 10, 5, 13, 10, 
	15, 15, 15, 10, 8, 9, 8, 8, 
	9, 17, 8, 10, 1, 3, 11, 12, 
	16, 10, 14, 10, 16, 10, 16, 10, 
	10, 10, 5, 10, 6, 7, 14, 10, 
	13, 10, 6, 14, 10, 15, 15, 15, 
	10, 16, 16, 16, 16, 10, 0, 10, 
	10, 10, 10, 10, 10, 10, 10, 10, 
	0
};

static const char _clang_trans_actions[] = {
	1, 15, 0, 0, 1, 0, 1, 13, 
	0, 0, 1, 0, 38, 0, 0, 33, 
	0, 0, 0, 33, 1, 0, 0, 1, 
	0, 3, 0, 35, 0, 0, 9, 9, 
	0, 11, 9, 11, 0, 11, 0, 11, 
	17, 19, 0, 21, 0, 0, 9, 25, 
	0, 27, 0, 9, 25, 0, 0, 0, 
	29, 0, 0, 0, 0, 23, 0, 31, 
	33, 33, 21, 25, 27, 25, 29, 23, 
	0
};

static const char _clang_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	5, 0, 5, 0, 0, 0, 0, 0, 
	0, 0
};

static const char _clang_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 7, 0, 0, 0, 0, 0, 
	0, 0
};

static const char _clang_eof_trans[] = {
	0, 0, 0, 0, 0, 64, 66, 66, 
	0, 0, 0, 67, 70, 69, 70, 71, 
	72, 0
};

static const int clang_start = 10;
static const int clang_error = 0;

static const int clang_en_c_comment = 8;
static const int clang_en_main = 10;


#line 94 "clang.rl"

#define BUFSIZE 128

void scanner()
{
	static char buf[BUFSIZE];
	int cs, act, have = 0, curline = 1;
	char *ts, *te = 0;
	int done = 0;

	
#line 126 "clang.c"
	{
	cs = clang_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 105 "clang.rl"

	while ( !done ) {
		char *p = buf + have, *pe, *eof = 0;
		int len, space = BUFSIZE - have;
		
		if ( space == 0 ) {
			/* We've used up the entire buffer storing an already-parsed token
			 * prefix that must be preserved. */
			fprintf(stderr, "OUT OF BUFFER SPACE\n" );
			exit(1);
		}

		len = fread( p, 1, space, stdin );
		pe = p + len;

		/* Check if this is the end of file. */
		if ( len < space ) {
			eof = pe;
			done = 1;
		}
			
		
#line 157 "clang.c"
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
	_acts = _clang_actions + _clang_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 3:
#line 1 "NONE"
	{ts = p;}
	break;
#line 178 "clang.c"
		}
	}

	_keys = _clang_trans_keys + _clang_key_offsets[cs];
	_trans = _clang_index_offsets[cs];

	_klen = _clang_single_lengths[cs];
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

	_klen = _clang_range_lengths[cs];
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
	cs = _clang_trans_targs[_trans];

	if ( _clang_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _clang_actions + _clang_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 12 "clang.rl"
	{curline += 1;}
	break;
	case 1:
#line 16 "clang.rl"
	{{cs = 10; goto _again;}}
	break;
	case 4:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 5:
#line 28 "clang.rl"
	{te = p+1;{
		printf( "symbol(%i): %c\n", curline, ts[0] );
	}}
	break;
	case 6:
#line 42 "clang.rl"
	{te = p+1;{
		printf( "single_lit(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 7:
#line 50 "clang.rl"
	{te = p+1;{
		printf( "double_lit(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 8:
#line 57 "clang.rl"
	{te = p+1;}
	break;
	case 9:
#line 62 "clang.rl"
	{te = p+1;}
	break;
	case 10:
#line 64 "clang.rl"
	{te = p+1;{ {cs = 8; goto _again;} }}
	break;
	case 11:
#line 28 "clang.rl"
	{te = p;p--;{
		printf( "symbol(%i): %c\n", curline, ts[0] );
	}}
	break;
	case 12:
#line 34 "clang.rl"
	{te = p;p--;{
		printf( "ident(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 13:
#line 68 "clang.rl"
	{te = p;p--;{
		printf( "int(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 14:
#line 76 "clang.rl"
	{te = p;p--;{
		printf( "float(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 15:
#line 84 "clang.rl"
	{te = p;p--;{
		printf( "hex(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
	case 16:
#line 28 "clang.rl"
	{{p = ((te))-1;}{
		printf( "symbol(%i): %c\n", curline, ts[0] );
	}}
	break;
	case 17:
#line 68 "clang.rl"
	{{p = ((te))-1;}{
		printf( "int(%i): ", curline );
		fwrite( ts, 1, te-ts, stdout );
		printf("\n");
	}}
	break;
#line 341 "clang.c"
		}
	}

_again:
	_acts = _clang_actions + _clang_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 2:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 354 "clang.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _clang_eof_trans[cs] > 0 ) {
		_trans = _clang_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 127 "clang.rl"

		if ( cs == clang_error ) {
			fprintf(stderr, "PARSE ERROR\n" );
			break;
		}

		if ( ts == 0 )
			have = 0;
		else {
			/* There is a prefix to preserve, shift it over. */
			have = pe - ts;
			memmove( buf, ts, have );
			te = buf + (te-ts);
			ts = buf;
		}
	}
}

int main()
{
	scanner();
	return 0;
}
