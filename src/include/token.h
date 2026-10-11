#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_STRUCT {
	enum {
		TOKEN_ID,
		TOKEN_EQUALS,
		TOKEN_STRING,
		TOKEN_NUMBER,
		// arithmetic + logical
		TOKEN_PLUS,
		TOKEN_MINUS,
		TOKEN_MULTIPLY,
		TOKEN_DIVIDE,
		TOKEN_EQUAL_EQUAL,
		TOKEN_LESS,
		TOKEN_MORE,
		TOKEN_LESS_THAN_EQUAL,
		TOKEN_MORE_THAN_EQUAL,
		TOKEN_NOT_EQUAL,
		TOKEN_NOT,
		TOKEN_SEMI,
		TOKEN_LEFTPAREN,
		TOKEN_RIGHTPAREN,
		TOKEN_LEFTBRACE,
		TOKEN_RIGHTBRACE,
		TOKEN_COMMA,
		TOKEN_EOF
	} type;
	
	char* value;
	
} token_T;

token_T* tokenInit(int type, char* value);
void tokenFree(token_T* token);

#endif
