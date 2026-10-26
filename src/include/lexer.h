#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "diagnostic.h"
#include <stddef.h>

typedef struct LEXER_STRUCT {
	char c;
	unsigned int index;
	char* contents;	// source code
	const char* fileName;
	
	size_t line;
	size_t column;
	
} lexer_T;

lexer_T* lexerInit(char* contents, const char* fileName);
void lexerFree(lexer_T* lexer);
void lexerAdvance(lexer_T* lexer);
void lexerSkipWhitespace(lexer_T* lexer);

token_T* lexerGetNextToken(lexer_T* lexer);
token_T* lexerCollectString(lexer_T* lexer);
token_T* lexerCollectNumber(lexer_T* lexer);
token_T* lexerCollectID(lexer_T* lexer);
token_T* lexerAdvanceWithToken(lexer_T* lexer, token_T* token);
char* lexerGetCurrCharAsStr(lexer_T* lexer);
#endif
