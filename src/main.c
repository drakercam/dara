#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitor.h"
#include "include/io.h"
#include "include/value.h"
#include "include/scope.h"
#include "include/builtin_registry.h"
#include "include/profiler.h"

#include <stdint.h>

void printHelp() {
	printf("Usage:\n dara <filename>\n");
}

void valuePrintSizes(void) {
    printf("sizeof(value_T): %zu\n", sizeof(value_T));
    printf("sizeof(int): %zu\n", sizeof(int));
    printf("sizeof(double): %zu\n", sizeof(double));
    printf("sizeof(float): %zu\n", sizeof(float));
    printf("sizeof(void*): %zu\n", sizeof(void*));

    printf("sizeof(uint8_t): %zu\n", sizeof(uint8_t));
    printf("sizeof(uint16_t): %zu\n", sizeof(uint16_t));
    printf("sizeof(uint32_t): %zu\n", sizeof(uint32_t));
    printf("sizeof(uint64_t): %zu\n", sizeof(uint64_t));
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printHelp();
		return 1;
	}
	
	lexer_T* lexer = lexerInit(
		fileGetContents(argv[1]), argv[1]
	);

	parser_T* parser = parserInit(lexer);
	builtinRegisterAll(parser->scope);
	
	parser->fileName = argv[1];
	ast_T* root = parserParse(parser, parser->scope);
	visitor_T* visitor = visitorInit();
	visitor->fileName = argv[1];

	value_T* result = visitorVisit(visitor, root);
	valueFree(result);
	visitorFree(visitor);
	astFree(root);
	parserFree(parser);
	lexerFree(lexer);
	
	#ifdef PROFILE
		profilerReport(stderr);
	#endif

	return 0;
}
