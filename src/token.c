#include "include/token.h"
#include <stdlib.h>

token_T* tokenInit(int type, char* value) {
	token_T* token = calloc(1, sizeof(token_T));
	token->type = type;
	token->value = value;
	
	return token;
}

void tokenFree(token_T* token) {
	if (token == NULL) {
		return;
	}
	
	free(token->value);
	free(token);
}
