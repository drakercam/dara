#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static bool parserLooksLikeTypedDeclaration(parser_T* parser) {
    lexer_T lookahead = *parser->lexer;

    token_T* token = lexerGetNextToken(&lookahead);
    bool result = false;

    // Ordinary declarations: int x = ...
    if (token->type == TOKEN_ID) {
        tokenFree(token);

        token = lexerGetNextToken(&lookahead);
        result = token->type == TOKEN_EQUALS;

        tokenFree(token);
        return result;
    }

    // Qualified declarations: std:array x = ...
    if (token->type != TOKEN_COLON) {
        tokenFree(token);
        return false;
    }

    tokenFree(token);

    // Consume the first namespace/construct segment.
    token = lexerGetNextToken(&lookahead);

    if (token->type != TOKEN_ID) {
        tokenFree(token);
        return false;
    }

    tokenFree(token);
    token = lexerGetNextToken(&lookahead);

    // Consume any additional qualified segments.
    while (token->type == TOKEN_COLON) {
        tokenFree(token);

        token = lexerGetNextToken(&lookahead);

        if (token->type != TOKEN_ID) {
            tokenFree(token);
            return false;
        }

        tokenFree(token);
        token = lexerGetNextToken(&lookahead);
    }

    // The next identifier is the variable name.
    if (token->type == TOKEN_ID) {
        tokenFree(token);

        token = lexerGetNextToken(&lookahead);
        result = token->type == TOKEN_EQUALS;
    }

    tokenFree(token);
    return result;
}


static bool parserLooksLikeAssignment(parser_T* parser) {
    lexer_T lookahead = *parser->lexer;
    token_T* token = lexerGetNextToken(&lookahead);

    // Skip a chain such as .health or .stats.health.
    while (token->type == TOKEN_DOT) {
        tokenFree(token);

        token = lexerGetNextToken(&lookahead);

        if (token->type != TOKEN_ID) {
            tokenFree(token);
            return false;
        }

        tokenFree(token);
        token = lexerGetNextToken(&lookahead);
    }

    bool result = token->type == TOKEN_EQUALS;

    tokenFree(token);

    return result;
}


parser_T* parserInit(lexer_T* lexer) {
	parser_T* parser = calloc(1, sizeof(parser_T));
	parser->lexer = lexer;
	parser->currentToken = lexerGetNextToken(lexer);
	parser->previousToken = (void*)0;
	
	parser->scope = scopeInit();
	
	return parser;
}

void parserFree(parser_T* parser) {
	if (parser == (void*)0) {
		return;
	}
	
	tokenFree(parser->previousToken);
	tokenFree(parser->currentToken);
	
	scopeFree(parser->scope);
	
	free(parser);
}

void parserEat(parser_T* parser, int tokenType) {
	if (parser->currentToken->type == tokenType) {
		tokenFree(parser->previousToken);
		
		parser->previousToken = parser->currentToken;
		parser->currentToken = lexerGetNextToken(parser->lexer);
	}
	else {
		printf("Unexpected token: '%s', with type %d\n", parser->currentToken->value, parser->currentToken->type);
		exit(1);
	}
}

ast_T* parserParse(parser_T* parser, scope_T* scope) {
	return parserParseStatements(parser, scope);
}

ast_T* parserParseStatement(parser_T* parser, scope_T* scope) {
    if (parser->currentToken->type == TOKEN_ID) {
        const char* value = parser->currentToken->value;

		if (strcmp(value, "if") == 0) {
			return parserParseIfStatement(parser, scope);
		}
		
		if (strcmp(value, "while") == 0) {
			return parserParseWhileLoop(parser, scope);
		}
		
		if (strcmp(value, "for") == 0) {
			return parserParseForLoop(parser, scope);
		}

        if (
            strcmp(value, "construct") == 0 ||
            strcmp(value, "namespace") == 0 ||
            strcmp(value, "var") == 0 ||
            strcmp(value, "auto") == 0 ||
            strcmp(value, "int") == 0 ||
            strcmp(value, "float") == 0 ||
            strcmp(value, "string") == 0 ||
            strcmp(value, "func") == 0 ||
            strcmp(value, "return") == 0
        ) {
            return parserParseID(parser, scope);
        }
        
        if (parserLooksLikeTypedDeclaration(parser)) {
			return parserParseVariableDefinition(parser, scope);
		}

		if (parserLooksLikeAssignment(parser)) {
			return parserParseAssignment(parser, scope);
		}

		return parserParseComparison(parser, scope);
    }

    return astInit(AST_NOOP);
}

ast_T* parserParseStatements(parser_T* parser, scope_T* scope) {
    ast_T* compound = astInit(AST_COMPOUND);
    compound->scope = scope;

    while (parser->currentToken->type != TOKEN_RIGHTBRACE && parser->currentToken->type != TOKEN_EOF) {
        ast_T* statement = parserParseStatement(parser, scope);

        if (statement == (void*)0) {
            break;
        }

        statement->scope = scope;

        size_t newSize = compound->compoundSize + 1;

        ast_T** newValue = realloc(
            compound->compoundValue,
            newSize * sizeof(ast_T*)
        );

        if (newValue == (void*)0) {
            astFree(statement);
            astFree(compound);
            printf("Failed to allocate compound statements\n");
            exit(EXIT_FAILURE);
        }

        compound->compoundValue = newValue;
        compound->compoundValue[newSize - 1] = statement;
        compound->compoundSize = newSize;

        if (parser->currentToken->type == TOKEN_SEMI) {
            parserEat(parser, TOKEN_SEMI);
        }
        else if (
            parser->currentToken->type != TOKEN_RIGHTBRACE &&
            parser->currentToken->type != TOKEN_EOF
        ) {
            printf("Expected ';' or end of block\n");
            astFree(compound);
            exit(EXIT_FAILURE);
        }
    }

    return compound;
}

ast_T* parserParseFactor(parser_T* parser, scope_T* scope) {
    switch (parser->currentToken->type) {
        case TOKEN_STRING:
            return parserParseString(parser, scope);

        case TOKEN_NUMBER:
            return parserParseNumber(parser, scope);

        case TOKEN_ID: {
			// Check whether this is a qualified construct name: std:array...
			lexer_T lookahead = *parser->lexer;
			token_T* nextToken = lexerGetNextToken(&lookahead);

			bool isQualifiedName = nextToken->type == TOKEN_COLON;
			tokenFree(nextToken);

			if (isQualifiedName) {
				char* qualifiedName =
					parserParseQualifiedFunctionName(parser);

				// A qualified name followed by '(' is a function call.
				if (parser->currentToken->type == TOKEN_LEFTPAREN) {
					ast_T* functionCall = parserParseFunctionCallNamed(
						parser,
						scope,
						qualifiedName
					);

					free(qualifiedName);
					return functionCall;
				}

				// Otherwise, preserve qualified construct method calls.
				if (scopeGetQualifiedConstructDefinition(scope, qualifiedName) == (void*)0) {
					printf("Unknown construct '%s'\n", qualifiedName);
					free(qualifiedName);
					exit(EXIT_FAILURE);
				}

				char* constructName = qualifiedName;

				parserEat(parser, TOKEN_DOT);

				if (parser->currentToken->type != TOKEN_ID) {
					printf("Expected method name after qualified construct\n");
					free(constructName);
					exit(EXIT_FAILURE);
				}

				ast_T* methodCall = astInit(AST_METHOD_CALL);
				methodCall->methodCallConstructName = constructName;

				methodCall->methodCallName = calloc(
					strlen(parser->currentToken->value) + 1,
					sizeof(char)
				);

				if (methodCall->methodCallName == (void*)0) {
					printf("Failed to allocate method name\n");
					astFree(methodCall);
					exit(EXIT_FAILURE);
				}

				strcpy(
					methodCall->methodCallName,
					parser->currentToken->value
				);

				parserEat(parser, TOKEN_ID);
				parserEat(parser, TOKEN_LEFTPAREN);

				methodCall->scope = scope;

				if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
					while (1) {
						ast_T* argument =
							parserParseComparison(parser, scope);

						size_t newSize =
							methodCall->methodCallArgumentsSize + 1;

						ast_T** newArguments = realloc(
							methodCall->methodCallArguments,
							newSize * sizeof(ast_T*)
						);

						if (newArguments == (void*)0) {
							printf("Failed to allocate method arguments\n");
							astFree(argument);
							astFree(methodCall);
							exit(EXIT_FAILURE);
						}

						methodCall->methodCallArguments = newArguments;
						methodCall->methodCallArguments[newSize - 1] = argument;
						methodCall->methodCallArgumentsSize = newSize;

						if (parser->currentToken->type != TOKEN_COMMA) {
							break;
						}

						parserEat(parser, TOKEN_COMMA);
					}
				}

				parserEat(parser, TOKEN_RIGHTPAREN);
				return methodCall;
			}

			
            ast_T* object;

            if (strcmp(parser->currentToken->value, "construct") == 0) {
                object = parserParseConstructInstantiation(parser, scope);
            }
            else if (
                strcmp(parser->currentToken->value, "true") == 0 ||
                strcmp(parser->currentToken->value, "false") == 0
            ) {
                object = astInit(AST_BOOLEAN);
                object->booleanValue =
                    strcmp(parser->currentToken->value, "true") == 0;
                object->scope = scope;

                parserEat(parser, TOKEN_ID);
            }
            else {
                object = parserParseID(parser, scope);
            }

            while (parser->currentToken->type == TOKEN_DOT) {
                parserEat(parser, TOKEN_DOT);

                if (parser->currentToken->type != TOKEN_ID) {
                    printf("Expected field or method name after '.'\n");
                    astFree(object);
                    exit(1);
                }

                char* fieldName = calloc(
                    strlen(parser->currentToken->value) + 1,
                    sizeof(char)
                );

                if (fieldName == (void*)0) {
                    printf("Failed to allocate member name\n");
                    astFree(object);
                    exit(EXIT_FAILURE);
                }

                strcpy(fieldName, parser->currentToken->value);

                // Consume the field or method name first.
                parserEat(parser, TOKEN_ID);

                // A name followed by '(' is a method call.
                if (parser->currentToken->type == TOKEN_LEFTPAREN) {
                    if (object->type != AST_VARIABLE) {
                        printf("Expected a construct name before method call\n");
                        free(fieldName);
                        astFree(object);
                        exit(1);
                    }

                    ast_T* methodCall = astInit(AST_METHOD_CALL);

                    methodCall->methodCallConstructName = calloc(
                        strlen(object->variableName) + 1,
                        sizeof(char)
                    );

                    if (methodCall->methodCallConstructName == (void*)0) {
                        printf("Failed to allocate construct name\n");
                        free(fieldName);
                        astFree(object);
                        astFree(methodCall);
                        exit(EXIT_FAILURE);
                    }

                    strcpy(
                        methodCall->methodCallConstructName,
                        object->variableName
                    );

                    methodCall->methodCallName = fieldName;
                    methodCall->scope = scope;

                    astFree(object);
                    object = (void*)0;

                    parserEat(parser, TOKEN_LEFTPAREN);

                    if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
                        while (1) {
                            ast_T* argument =
                                parserParseComparison(parser, scope);

                            size_t newSize =
                                methodCall->methodCallArgumentsSize + 1;

                            ast_T** newArguments = realloc(
                                methodCall->methodCallArguments,
                                newSize * sizeof(ast_T*)
                            );

                            if (newArguments == (void*)0) {
                                printf("Failed to allocate method arguments\n");
                                astFree(argument);
                                astFree(methodCall);
                                exit(EXIT_FAILURE);
                            }

                            methodCall->methodCallArguments = newArguments;
                            methodCall->methodCallArguments[newSize - 1] =
                                argument;
                            methodCall->methodCallArgumentsSize = newSize;

                            if (parser->currentToken->type != TOKEN_COMMA) {
                                break;
                            }

                            parserEat(parser, TOKEN_COMMA);
                        }
                    }

                    parserEat(parser, TOKEN_RIGHTPAREN);

                    object = methodCall;
                }
                else {
                    ast_T* member = astInit(AST_MEMBER_ACCESS);

                    member->memberAccessObject = object;
                    member->memberAccessField = fieldName;
                    member->scope = scope;

                    object = member;
                }
            }

            return object;
        }

        case TOKEN_LEFTPAREN: {
            parserEat(parser, TOKEN_LEFTPAREN);

            ast_T* expression = parserParseComparison(parser, scope);

            parserEat(parser, TOKEN_RIGHTPAREN);
            return expression;
        }

        default:
            printf(
                "Unexpected token in expression: '%s', with type %d\n",
                parser->currentToken->value,
                parser->currentToken->type
            );
            exit(1);
    }
}

ast_T* parserParseID(parser_T* parser, scope_T* scope) {
	
	if (strcmp(parser->currentToken->value, "namespace") == 0) {
		return parserParseNamespaceDefinition(parser, scope);
	}
	else if (strcmp(parser->currentToken->value, "construct") == 0) {
		return parserParseConstructDefinition(parser, scope);
	}
	else if (
		strcmp(parser->currentToken->value, "var") == 0 ||
		strcmp(parser->currentToken->value, "auto") == 0 ||
		strcmp(parser->currentToken->value, "int") == 0 ||
		strcmp(parser->currentToken->value, "string") == 0 ||
		strcmp(parser->currentToken->value, "float") == 0
	) {
		return parserParseVariableDefinition(parser, scope);
	}
	else if (strcmp(parser->currentToken->value, "func") == 0) {
		return parserParseFunctionDefinition(parser, scope);
	}
	else if (strcmp(parser->currentToken->value, "return") == 0) {
		return parserParseReturnStatement(parser, scope);
	}
	else {
		return parserParseVariable(parser, scope);
	}
}
