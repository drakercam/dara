#include "../include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Parses a namespace declaration, including its body, and registers
 * the resulting namespace definition in the supplied scope.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope in which the namespace is declared.
 *
 * Returns:
 *   The AST node representing the namespace definition.
 *
 * Terminates the program if the namespace name is invalid,
 * allocation fails, or registration fails.
 */

ast_T* parserParseNamespaceDefinition(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_NAMESPACE_DEFINITION);
    parserSetNodeLocation(ast, parser->currentToken);
    ast->scope = scope;

    parserEat(parser, TOKEN_ID); // namespace keyword

    if (parser->currentToken->type != TOKEN_ID) {
		parserReportError(parser, "E1118", "Expected namespace name.");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    ast->namespaceDefinitionName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));

    if (!ast->namespaceDefinitionName) {
        printf("Failed to allocate namespace name\n");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    strcpy(ast->namespaceDefinitionName, parser->currentToken->value);

    parserEat(parser, TOKEN_ID); // namespace name
    parserEat(parser, TOKEN_LEFTBRACE);
    
    ast_T* existing = scopeGetNamespaceDefinition(scope, ast->namespaceDefinitionName);
    if (!existing) {
		ast->namespaceDefinitionScope = scopeInit();

		ast->namespaceDefinitionBody = parserParseStatements(
			parser,
			ast->namespaceDefinitionScope
		);

		parserEat(parser, TOKEN_RIGHTBRACE);

		if (scopeAddNamespaceDefinition(scope, ast) == (void*)0) {
			printf(
				"Failed to register namespace '%s'\n",
				ast->namespaceDefinitionName
			);
			astFree(ast);
			exit(EXIT_FAILURE);
		}

		return ast;
	}
	
	// namespace already exists so parse the new block into its existing scope
	ast->namespaceDefinitionScope = (void*)0;
	
	ast_T* newBody = parserParseStatements(parser, existing->namespaceDefinitionScope);
	parserEat(parser, TOKEN_RIGHTBRACE);
	
	ast_T* oldBody = existing->namespaceDefinitionBody;
	
	size_t oldSize = oldBody->compoundSize;
	size_t newSize = newBody->compoundSize;
	size_t totalSize = oldSize + newSize;
	
	if (newSize > 0) {
		ast_T** statements = realloc(oldBody->compoundValue, totalSize * sizeof(ast_T*));
		
		if (!statements) {
			astFree(newBody);
			astFree(ast);
			printf("Failed to append namespace statements.\n");
			exit(EXIT_FAILURE);
		}
		
		oldBody->compoundValue = statements;
		
		for (size_t i = 0; i < newSize; ++i) {
			oldBody->compoundValue[oldSize + i] = newBody->compoundValue[i];
		}
		
		oldBody->compoundSize = totalSize;
	}
	
	// the statements now belong to old body, free only the temporary compound array and shell.
	free(newBody->compoundValue);
	free(newBody);
	ast->namespaceDefinitionBody = (void*)0;
	
	astFree(ast);
	
	return astInit(AST_NOOP);
}
