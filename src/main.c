#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitor.h"
#include "include/io.h"
#include "include/value.h"
#include "include/scope.h"

void printHelp() {
	printf("Usage:\n dara <filename>\n");
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printHelp();
		return 1;
	}

	lexer_T* lexer = lexerInit(
		fileGetContents(argv[1])
	);

	parser_T* parser = parserInit(lexer);
	ast_T* root = parserParse(parser, parser->scope);
	visitor_T* visitor = visitorInit();

	visitorVisit(visitor, root);

	visitorFree(visitor);
	astFree(root);
	parserFree(parser);
	lexerFree(lexer);

	return 0;
}
