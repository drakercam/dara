#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_STRUCT {
	enum {
		TOKEN_ID,
		TOKEN_EQUALS,
		TOKEN_STRING,
		TOKEN_NUMBER,
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
