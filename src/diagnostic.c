#include "include/diagnostic.h"

#include <stdio.h>
#include <stdlib.h>

diagnostic_T* diagnosticInit(diagnosticCategory_T category, char* code, char* message, sourceLocation_T location) {
	diagnostic_T* diagnostic = calloc(1, sizeof(diagnostic_T));
	
	if (diagnostic == (void*)0) {
		return (void*)0;
	}
	
	diagnostic->category = category;
	diagnostic->code = code;
	diagnostic->message = message;
	diagnostic->location = location;

	return diagnostic;
}

void diagnosticPrint(const diagnostic_T* diagnostic, const char* filename) {
	
	if (diagnostic == (void*)0) {
		return;
	}
	
	const char* categoryName = "Error";
	
	switch (diagnostic->category) {
		case DIAGNOSTIC_LEXER:
			categoryName = "Lexical Error";
			break;
		case DIAGNOSTIC_PARSER:
			categoryName = "Syntax Error";
			break;
		case DIAGNOSTIC_RUNTIME:
			categoryName = "Runtime Error";
			break;
	}
	
	fprintf(
		stderr,
		"%s:%zu:%zu: %s[%s]: %s\n",
		filename != (void*)0 ? filename : "<source>",
		diagnostic->location.line,
		diagnostic->location.column,
		categoryName,
		diagnostic->code != (void*)0 ? diagnostic->code : "E0000",
		diagnostic->message != (void*)0 ? diagnostic->message : "Unknown error"
	);
}

void diagnosticFree(diagnostic_T* diagnostic) {
	if (diagnostic == (void*)0) {
		return;
	}
	
	free(diagnostic->code);
	free(diagnostic->message);
	free(diagnostic);
}
