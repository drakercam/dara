#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitor.h"
#include "include/io.h"
#include "include/value.h"

void printHelp() {
	printf("Usage:\n dara <filename>\n");
}

int main(int argc, char* argv[]) {
	//if (argc < 2) {
	//	printHelp();
	//}
	
	//lexer_T* lexer = lexerInit(
	//	fileGetContents(argv[1])
	//);
	
	value_T* player = valueInitStruct();

	valueStructSetField(
		player,
		"name",
		valueInitString("Draker")
	);

	valueStructSetField(
		player,
		"health",
		valueInitNumber(100)
	);

	value_T* health =
		valueStructGetField(player, "health");

	printf("Health: %f\n", health->numberValue);

	valueStructSetField(
		player,
		"health",
		valueInitNumber(200)
	);

	health =
		valueStructGetField(player, "health");

	printf("Health: %f\n", health->numberValue);

	valueFree(player);
	
	//parser_T* parser = parserInit(lexer);
	//ast_T* root = parserParse(parser, parser->scope);
	//visitor_T* visitor = visitorInit();
	
	//visitorVisit(visitor, root);
	
	//visitorFree(visitor);
	//astFree(root);
	//parserFree(parser);
	//lexerFree(lexer);

	return 0;
}
