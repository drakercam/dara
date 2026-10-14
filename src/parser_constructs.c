#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * Checks whether a name identifies a construct type
 * registered in the given scope.
 *
 * Parameters:
 *   scope - Scope in which to search for the construct.
 *   name  - Name of the construct type to check.
 *
 * Returns:
 *   true if the construct exists; otherwise false.
 */
 
 static bool parserIsConstructType(scope_T* scope, const char* name) {
    //~ printf("Checking type: %s (known constructs: %zu)\n",
        //~ name, scope->constructDefinitionsSize);

    return scopeGetConstructDefinition(scope, name) != (void*)0;
}

/*
 * Parses a construct name that may contain nested namespace
 * qualification, such as io:types:Player.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to validate the qualified construct.
 *
 * Returns:
 *   A dynamically allocated string containing the construct name.
 *   The caller is responsible for freeing the returned string.
 *
 * Terminates the program if the name is malformed, allocation
 * fails, or the qualified construct cannot be resolved.
 */
 
 char* parserParseQualifiedConstructName(parser_T* parser, scope_T* scope) {
    if (parser->currentToken->type != TOKEN_ID) {
        printf("Expected construct name\n");
        exit(EXIT_FAILURE);
    }

    size_t nameLength = strlen(parser->currentToken->value);

    char* name = calloc(nameLength + 1, sizeof(char));

    if (name == (void*)0) {
        printf("Failed to allocate construct name\n");
        exit(EXIT_FAILURE);
    }

    strcpy(name, parser->currentToken->value);
    parserEat(parser, TOKEN_ID);

    // Unqualified construct names, such as Human.
    if (parser->currentToken->type != TOKEN_COLON) {
        return name;
    }

    // Read each remaining segment of the qualified name.
    while (parser->currentToken->type == TOKEN_COLON) {
        parserEat(parser, TOKEN_COLON);

        if (parser->currentToken->type != TOKEN_ID) {
            printf("Expected name after ':'\n");
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
            printf("Failed to allocate qualified construct name\n");
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

    // Validate the complete path, including every nested namespace.
    if (scopeGetQualifiedConstructDefinition(scope, name) == (void*)0) {
        printf("Unknown construct '%s'\n", name);
        free(name);
        exit(EXIT_FAILURE);
    }

    return name;
}

/*
 * Parses a construct declaration, including its fields and methods,
 * and registers the resulting definition in the supplied scope.
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope in which the construct is defined.
 *
 * Returns:
 *   The AST node representing the construct definition.
 */
 
 ast_T* parserParseConstructDefinition(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_CONSTRUCT_DEFINITION);
	ast->scope = scope;
	
	parserEat(parser, TOKEN_ID);	// construct keyword
	
	// store the construct name
	if (parser->currentToken->type != TOKEN_ID) {
		printf("Expected construct name\n");
		exit(1);
	}
	
	ast->constructDefinitionName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
	strcpy(ast->constructDefinitionName, parser->currentToken->value);
	
	parserEat(parser, TOKEN_ID);
	
	// begin construct body
	parserEat(parser, TOKEN_LEFTBRACE);
	
	// Parse fields and methods inside the construct.
	while (parser->currentToken->type != TOKEN_RIGHTBRACE) {
		if (parser->currentToken->type != TOKEN_ID) {
			printf("Expected field or method declaration\n");
			exit(1);
		}

		// Method declaration: func name(...) -> type { ... }
		if (strcmp(parser->currentToken->value, "func") == 0) {
			ast_T* method = parserParseFunctionDefinition(parser, scope);
			
			if (parser->currentToken->type == TOKEN_SEMI) {
				parserEat(parser, TOKEN_SEMI);
			}

			size_t newSize = ast->constructDefinitionMethodsSize + 1;

			struct AST_STRUCT** newMethods = realloc(
				ast->constructDefinitionMethods,
				newSize * sizeof(ast_T*)
			);

			if (newMethods == (void*)0) {
				printf("Failed to allocate construct methods\n");
				astFree(method);
				astFree(ast);
				exit(EXIT_FAILURE);
			}

			ast->constructDefinitionMethods = newMethods;
			ast->constructDefinitionMethods[newSize - 1] = method;
			ast->constructDefinitionMethodsSize = newSize;

			// A method declaration is complete. Parse the next item.
			continue;
		}

		// Otherwise, parse a field declaration as before.

		// Store the declared type
		char* fieldType = parser->currentToken->value;

		if (strcmp(fieldType, "var") != 0 &&
			strcmp(fieldType, "auto") != 0 &&
			strcmp(fieldType, "int") != 0 &&
			strcmp(fieldType, "float") != 0 &&
			strcmp(fieldType, "string") != 0 &&
			!parserIsConstructType(scope, fieldType)) {
			printf("Unknown field type '%s'\n", fieldType);
			exit(1);
		}

		// Preserve var as an alias for auto
		if (strcmp(fieldType, "var") == 0)
			fieldType = "auto";

		char* fieldTypeCopy = calloc(strlen(fieldType) + 1, sizeof(char));
		if (fieldTypeCopy == NULL) {
			printf("Failed to allocate field type\n");
			exit(EXIT_FAILURE);
		}
		strcpy(fieldTypeCopy, fieldType);

		parserEat(parser, TOKEN_ID);

		if (parser->currentToken->type != TOKEN_ID) {
			printf("Expected field name\n");
			free(fieldTypeCopy);
			exit(1);
		}

		char* fieldName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
		if (fieldName == (void*)0) {
			printf("Failed to allocate field name\n");
			free(fieldTypeCopy);
			exit(EXIT_FAILURE);
		}
		strcpy(fieldName, parser->currentToken->value);

		parserEat(parser, TOKEN_ID);
		parserEat(parser, TOKEN_SEMI);

		size_t newSize = ast->constructDefinitionFieldsSize + 1;

		char** newFields = realloc(
			ast->constructDefinitionFields,
			newSize * sizeof(char*)
		);

		if (newFields == (void*)0) {
			printf("Failed to allocate construct fields\n");
			free(fieldName);
			free(fieldTypeCopy);
			exit(EXIT_FAILURE);
		}
		ast->constructDefinitionFields = newFields;

		char** newTypes = realloc(
			ast->constructDefinitionFieldTypes,
			newSize * sizeof(char*)
		);

		if (newTypes == (void*)0) {
			printf("Failed to allocate construct field types\n");
			free(fieldName);
			free(fieldTypeCopy);
			exit(EXIT_FAILURE);
		}
		ast->constructDefinitionFieldTypes = newTypes;

		ast->constructDefinitionFields[ast->constructDefinitionFieldsSize] = fieldName;
		ast->constructDefinitionFieldTypes[ast->constructDefinitionFieldsSize] = fieldTypeCopy;
		ast->constructDefinitionFieldsSize = newSize;
	}
	
	parserEat(parser, TOKEN_RIGHTBRACE);
	
	if (scopeAddConstructDefinition(scope, ast) == (void*)0) {
		printf("Failed to register construct '%s'\n",
			ast->constructDefinitionName);
		astFree(ast);
		exit(EXIT_FAILURE);
	}
	
	//~ printf("Registered construct: %s (total: %zu)\n", ast->constructDefinitionName, scope->constructDefinitionsSize);
	
	return ast;
}

/*
 * Parses a construct instantiation and its designated field
 * initializers, such as construct Player(.x = 10).
 *
 * Parameters:
 *   parser - Parser containing the current token stream.
 *   scope  - Scope used to parse the initializer expressions.
 *
 * Returns:
 *   The AST node representing the construct instantiation.
 */

ast_T* parserParseConstructInstantiation(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_CONSTRUCT_INSTANTIATION);
    ast->scope = scope;

    parserEat(parser, TOKEN_ID); // consume "construct"

    ast->constructInstantiationName =
        parserParseQualifiedConstructName(parser, scope);

    parserEat(parser, TOKEN_LEFTPAREN);

    if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
        while (1) {
            // Each initializer starts with a dot.
            parserEat(parser, TOKEN_DOT);

            if (parser->currentToken->type != TOKEN_ID) {
                printf("Expected field name after '.'\n");
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            char* fieldName = calloc(
                strlen(parser->currentToken->value) + 1,
                sizeof(char)
            );

            if (fieldName == (void*)0) {
                printf("Failed to allocate initializer field name\n");
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            strcpy(fieldName, parser->currentToken->value);
            parserEat(parser, TOKEN_ID);
            parserEat(parser, TOKEN_EQUALS);

            ast_T* fieldValue = parserParseComparison(parser, scope);

            size_t newSize =
                ast->constructInstantiationFieldsSize + 1;

            char** newNames = realloc(
                ast->constructInstantiationFieldNames,
                newSize * sizeof(char*)
            );

            if (newNames == (void*)0) {
                printf("Failed to allocate initializer names\n");
                free(fieldName);
                astFree(fieldValue);
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            ast->constructInstantiationFieldNames = newNames;

            ast_T** newValues = realloc(
                ast->constructInstantiationFieldValues,
                newSize * sizeof(ast_T*)
            );

            if (newValues == (void*)0) {
                printf("Failed to allocate initializer values\n");
                free(fieldName);
                astFree(fieldValue);
                astFree(ast);
                exit(EXIT_FAILURE);
            }

            ast->constructInstantiationFieldValues = newValues;
            ast->constructInstantiationFieldNames[newSize - 1] = fieldName;
            ast->constructInstantiationFieldValues[newSize - 1] = fieldValue;
            ast->constructInstantiationFieldsSize = newSize;

            if (parser->currentToken->type != TOKEN_COMMA) {
                break;
            }

            parserEat(parser, TOKEN_COMMA);
        }
    }

    parserEat(parser, TOKEN_RIGHTPAREN);

    return ast;
}
