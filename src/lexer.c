#include "include/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define NEW_LINE 10

lexer_T* lexerInit(char* contents) {
	lexer_T* lexer = calloc(1, sizeof(struct LEXER_STRUCT));
	
	lexer->contents = contents;
	lexer->index = 0;
	lexer->c = contents[lexer->index];
	
	return lexer;
}

void lexerFree(lexer_T* lexer) {
	if (lexer == (void*)0) {
		return;
	}
	
	free(lexer->contents);
	free(lexer);
}

void lexerAdvance(lexer_T* lexer) {
	if (lexer->c != '\0' && lexer->index < strlen(lexer->contents)) {
		lexer->index += 1;
		lexer->c = lexer->contents[lexer->index];
	}
}

void lexerSkipWhitespace(lexer_T* lexer) {
	while (isspace((unsigned char)lexer->c)) {	// skip new lines too
		lexerAdvance(lexer);
	}
}

token_T* lexerGetNextToken(lexer_T* lexer) {
	while (lexer->c != '\0' && lexer->index < strlen(lexer->contents)) {
		if (lexer->c == ' ' || lexer->c == NEW_LINE) {
			lexerSkipWhitespace(lexer);
			continue;
		}
		
		if (isdigit((unsigned char)lexer->c))
			return lexerCollectNumber(lexer);

		if (isalpha((unsigned char)lexer->c))
			return lexerCollectID(lexer);
		
		if (lexer->c == '"') {
			return lexerCollectString(lexer);
		}
		
		switch (lexer->c) {
			case '=':
				if (lexer->contents[lexer->index + 1] == '=') {
					char* value = calloc(3, sizeof(char));
					value[0] = '=';
					value[1] = '=';
					
					lexerAdvance(lexer);
					return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_EQUAL_EQUAL, value));
				}
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_EQUALS, lexerGetCurrCharAsStr(lexer)));
				break;
			case ';':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_SEMI, lexerGetCurrCharAsStr(lexer)));
				break;
			case '(':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_LEFTPAREN, lexerGetCurrCharAsStr(lexer)));
				break;
			case ')':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_RIGHTPAREN, lexerGetCurrCharAsStr(lexer)));
				break;
			case '{':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_LEFTBRACE, lexerGetCurrCharAsStr(lexer)));
				break;
			case '}':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_RIGHTBRACE, lexerGetCurrCharAsStr(lexer)));
				break;
			case ',':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_COMMA, lexerGetCurrCharAsStr(lexer)));
				break;
			case '+':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_PLUS, lexerGetCurrCharAsStr(lexer)));
				break;
			case '-':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_MINUS, lexerGetCurrCharAsStr(lexer)));
				break;
			case '*':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_MULTIPLY, lexerGetCurrCharAsStr(lexer)));
				break;
			case '/':
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_DIVIDE, lexerGetCurrCharAsStr(lexer)));
				break;
			case '<':	// handle < and <=
				if (lexer->contents[lexer->index + 1] == '=') {
					char* value = calloc(3, sizeof(char));
					value[0] = '<';
					value[1] = '=';
					
					lexerAdvance(lexer);
					return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_LESS_THAN_EQUAL, value));
				}
			
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_LESS, lexerGetCurrCharAsStr(lexer)));
				break;
			case '>':	// handle > and >=
				if (lexer->contents[lexer->index + 1] == '=') {
					char* value = calloc(3, sizeof(char));
					value[0] = '>';
					value[1] = '=';
					
					lexerAdvance(lexer);
					return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_MORE_THAN_EQUAL, value));
				}
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_MORE, lexerGetCurrCharAsStr(lexer)));
				break;
				
			case '!':	// handle ! and !=
				if (lexer->contents[lexer->index + 1] == '=') {
					char* value = calloc(3, sizeof(char));
					value[0] = '!';
					value[1] = '=';
					
					lexerAdvance(lexer);
					return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_NOT_EQUAL, value));
				}
				return lexerAdvanceWithToken(lexer, tokenInit(TOKEN_NOT, lexerGetCurrCharAsStr(lexer)));
				break;
			
			default:
				printf("Unknown character: '%c' (%d)\n", lexer->c, lexer->c);
				lexerAdvance(lexer);
				break;
		}
	}
	
	return tokenInit(TOKEN_EOF, calloc(1, 1));
}

token_T* lexerCollectString(lexer_T* lexer) {
	lexerAdvance(lexer);	// skip the quotes we are encountering
	
	char* value = calloc(1, sizeof(char));
	value[0] = '\0';
	
	while (lexer->c != '"') {
		char* s = lexerGetCurrCharAsStr(lexer);
		value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
		strcat(value, s);
		free(s);
		
		lexerAdvance(lexer);
	}
	
	lexerAdvance(lexer);    // skip closing quote
		
	return tokenInit(TOKEN_STRING, value);
}

token_T* lexerCollectNumber(lexer_T* lexer) {
	char* value = calloc(1, sizeof(char));
    value[0] = '\0';

    while (isdigit((unsigned char)lexer->c)) {
        char* s = lexerGetCurrCharAsStr(lexer);

        value = realloc(
            value,
            (strlen(value) + strlen(s) + 1) * sizeof(char)
        );

        strcat(value, s);
        free(s);

        lexerAdvance(lexer);
    }

    return tokenInit(TOKEN_NUMBER, value);
}

token_T* lexerCollectID(lexer_T* lexer) {	
	char* value = calloc(1, sizeof(char));
	value[0] = '\0';
	
	while (isalnum(lexer->c)) {	// while the character is alphanumeric
		char* s = lexerGetCurrCharAsStr(lexer);
		value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
		strcat(value, s);
		free(s);
		
		lexerAdvance(lexer);
	}
		
	return tokenInit(TOKEN_ID, value);
}

token_T* lexerAdvanceWithToken(lexer_T* lexer, token_T* token) {
	lexerAdvance(lexer);
	
	return token;
}

char* lexerGetCurrCharAsStr(lexer_T* lexer) {
	char* str = calloc(2, sizeof(char));
	str[0] = lexer->c;
	str[1] = '\0';
	
	return str;
}
