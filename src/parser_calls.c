#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
 * Parses a function name that may contain namespace qualification,
 * such as io:debug:print.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *
 * Returns:
 *   A dynamically allocated string containing the qualified name.
 *   The caller is responsible for freeing the returned string.
 *
 * Terminates the program if the name is malformed or allocation
 * fails.
 */
 
 char* parserParseQualifiedFunctionName(parser_T* parser) {
    if (parser->currentToken->type != TOKEN_ID) {
        parserReportError(parser, "E1106", "Expected a function name.");
		exit(EXIT_FAILURE);
    }

    size_t nameLength = strlen(parser->currentToken->value);
    char* name = calloc(nameLength + 1, sizeof(char));

    if (name == (void*)0) {
        printf("Failed to allocate function name\n");
        exit(EXIT_FAILURE);
    }

    strcpy(name, parser->currentToken->value);
    parserEat(parser, TOKEN_ID);

    while (parser->currentToken->type == TOKEN_COLON) {
        parserEat(parser, TOKEN_COLON);

        if (parser->currentToken->type != TOKEN_ID) {
            parserReportError(parser, "E1107", "Expected an identifier after ':'.");
			free(name);
			exit(EXIT_FAILURE);
        }

        const char* segment = parser->currentToken->value;
        size_t segmentLength = strlen(segment);

        char* expandedName = realloc(
            name,
            nameLength + segmentLength + 2
        );

        if (expandedName == (void*)0) {
            printf("Failed to allocate qualified function name\n");
            free(name);
            exit(EXIT_FAILURE);
        }

        name = expandedName;
        name[nameLength] = ':';

        memcpy(
            name + nameLength + 1,
            segment,
            segmentLength + 1
        );

        nameLength += segmentLength + 1;

        parserEat(parser, TOKEN_ID);
    }

    return name;
}

/*
 * Parses a function call using a name that has already been parsed,
 * including its arguments.
 *
 * Parameters:
 *   parser       - Parser containing the current token stream.
 *   scope        - Scope used to parse the call's arguments.
 *   functionName - Name of the function being called.
 *
 * Returns:
 *   The AST node representing the function call.
 */

ast_T* parserParseFunctionCallNamed(parser_T* parser, scope_T* scope, const char* functionName) {
    ast_T* functionCall = astInit(AST_FUNCTION_CALL);

    functionCall->functionCallName = calloc(
        strlen(functionName) + 1,
        sizeof(char)
    );

    if (functionCall->functionCallName == (void*)0) {
        printf("Failed to allocate function call name\n");
        astFree(functionCall);
        exit(EXIT_FAILURE);
    }

    strcpy(functionCall->functionCallName, functionName);

    parserEat(parser, TOKEN_LEFTPAREN);

    if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
        while (1) {
            ast_T* argument = parserParseComparison(parser, scope);

            size_t newSize =
                functionCall->functionCallArgumentsSize + 1;

            ast_T** newArguments = realloc(
                functionCall->functionCallArguments,
                newSize * sizeof(ast_T*)
            );

            if (newArguments == (void*)0) {
                printf("Failed to allocate function arguments\n");
                astFree(argument);
                astFree(functionCall);
                exit(EXIT_FAILURE);
            }

            functionCall->functionCallArguments = newArguments;
            functionCall->functionCallArguments[newSize - 1] = argument;
            functionCall->functionCallArgumentsSize = newSize;

            if (parser->currentToken->type != TOKEN_COMMA) {
                break;
            }

            parserEat(parser, TOKEN_COMMA);
        }
    }

    parserEat(parser, TOKEN_RIGHTPAREN);
    functionCall->scope = scope;

    return functionCall;
}

/*
 * Parses an unqualified function call using the previously consumed
 * function-name token.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the call's arguments.
 *
 * Returns:
 *   The AST node representing the function call.
 */

ast_T* parserParseFunctionCall(parser_T* parser, scope_T* scope) {
    return parserParseFunctionCallNamed(
        parser,
        scope,
        parser->previousToken->value
    );
}
