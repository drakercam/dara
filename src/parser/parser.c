#include "../include/parser.h"
#include "../include/diagnostic.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


/*
 * Reports a parser diagnostic at the current token's location.
 */
void parserReportError(parser_T* parser, const char* code, const char* message) {
    char* ownedCode = calloc(strlen(code) + 1, sizeof(char));
    char* ownedMessage = calloc(strlen(message) + 1, sizeof(char));

    if (ownedCode == (void*)0 || ownedMessage == (void*)0) {
        free(ownedCode);
        free(ownedMessage);
        fprintf(stderr, "Failed to allocate diagnostic.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ownedCode, code);
    strcpy(ownedMessage, message);

    diagnostic_T* diagnostic = diagnosticInit(
        DIAGNOSTIC_PARSER,
        ownedCode,
        ownedMessage,
        parser->currentToken->location
    );

    if (diagnostic == (void*)0) {
        free(ownedCode);
        free(ownedMessage);
        fprintf(stderr, "Failed to allocate diagnostic.\n");
        exit(EXIT_FAILURE);
    }

    diagnosticPrint(diagnostic, parser->fileName);
    diagnosticFree(diagnostic);
}

/*
 * Copies a token's source location to an AST node.
 * The token must remain valid while this function runs.
 */
void parserSetNodeLocation(ast_T* node, const token_T* token) {
    if (node == (void*)0 || token == (void*)0) {
        return;
    }

    node->location = token->location;
}

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

    bool result =
    token->type == TOKEN_EQUALS ||
    token->type == TOKEN_PLUS_EQUAL ||
    token->type == TOKEN_MINUS_EQUAL;

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


/*
 * Returns a human-readable name for a token type.
 */
static const char* parserTokenTypeName(int tokenType) {
    switch (tokenType) {
        case TOKEN_ID:              return "identifier";
        case TOKEN_EQUALS:          return "'='";
        case TOKEN_STRING:          return "string literal";
        case TOKEN_NUMBER:          return "number";
        case TOKEN_PLUS:            return "'+'";
        case TOKEN_MINUS:           return "'-'";
        case TOKEN_PLUS_EQUAL:      return "'+='";
        case TOKEN_MINUS_EQUAL:     return "'-='";
        case TOKEN_ARROW:           return "'->'";
        case TOKEN_MULTIPLY:        return "'*'";
        case TOKEN_DIVIDE:          return "'/'";
        case TOKEN_EQUAL_EQUAL:     return "'=='";
        case TOKEN_LESS:            return "'<'";
        case TOKEN_MORE:            return "'>'";
        case TOKEN_LESS_THAN_EQUAL: return "'<='";
        case TOKEN_MORE_THAN_EQUAL: return "'>='";
        case TOKEN_NOT_EQUAL:       return "'!='";
        case TOKEN_NOT:             return "'!'";
        case TOKEN_SEMI:            return "';'";
        case TOKEN_LEFTPAREN:       return "'('";
        case TOKEN_RIGHTPAREN:      return "')'";
        case TOKEN_LEFTBRACE:       return "'{'";
        case TOKEN_RIGHTBRACE:      return "'}'";
        case TOKEN_COMMA:           return "','";
        case TOKEN_COLON:           return "':'";
        case TOKEN_DOT:             return "'.'";
        case TOKEN_AMPERSAND:       return "'&'";
        case TOKEN_EOF:             return "end of file";
        default:                    return "unknown token";
    }
}

/*
 * Consumes the current token if it matches the expected type.
 * Otherwise, reports a syntax diagnostic and terminates parsing.
 */
void parserEat(parser_T* parser, int tokenType) {
    if (parser->currentToken->type == tokenType) {
        tokenFree(parser->previousToken);
        parser->previousToken = parser->currentToken;
        parser->currentToken = lexerGetNextToken(parser->lexer);
        return;
    }

        char message[512];

    snprintf(
        message,
        sizeof(message),
        "Expected %s, but found %s%s%s.",
        parserTokenTypeName(tokenType),
        parser->currentToken->value != (void*)0 ? "'" : "",
        parser->currentToken->value != (void*)0
            ? parser->currentToken->value
            : parserTokenTypeName(parser->currentToken->type),
        parser->currentToken->value != (void*)0 ? "'" : ""
    );

    parserReportError(parser, "E1101", message);
    exit(EXIT_FAILURE);
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
            strcmp(value, "array") == 0 ||
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

		ast_T* expression = parserParseComparison(parser, scope);

		if (parser->currentToken->type == TOKEN_EQUALS ||
			parser->currentToken->type == TOKEN_PLUS_EQUAL ||
			parser->currentToken->type == TOKEN_MINUS_EQUAL) {
				
			parserReportError(parser, "E1103", "Invalid assignment target.");

			astFree(expression);
			exit(EXIT_FAILURE);
		}

		return expression;
    }

    return parserParseComparison(parser, scope);
}

ast_T* parserParseStatements(parser_T* parser, scope_T* scope) {
    ast_T* compound = astInit(AST_COMPOUND);
    parserSetNodeLocation(compound, parser->currentToken);
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
		else if (parser->currentToken->type == TOKEN_EQUALS ||
			parser->currentToken->type == TOKEN_PLUS_EQUAL ||
			parser->currentToken->type == TOKEN_MINUS_EQUAL) {
				
			parserReportError(parser, "E1103", "Invalid assignment target.");
			astFree(compound);
			exit(EXIT_FAILURE);
		}
		else if (parser->currentToken->type != TOKEN_RIGHTBRACE &&
			parser->currentToken->type != TOKEN_EOF) {
				
			parserReportError(parser,"E1105", "Expected ';' or End of Block.");
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
			sourceLocation_T startLocation = parser->currentToken->location;
			lexer_T lookahead = *parser->lexer;
			token_T* nextToken = lexerGetNextToken(&lookahead);

			bool isQualifiedName = nextToken->type == TOKEN_COLON;
			tokenFree(nextToken);

			if (isQualifiedName) {
				char* qualifiedName =
					parserParseQualifiedFunctionName(parser);

				// A qualified name followed by '(' is a function call.
				if (parser->currentToken->type == TOKEN_LEFTPAREN) {
					if (strcmp(qualifiedName, "std:array") == 0) {
						free(qualifiedName);
						return parserParseArrayConstructor(parser, scope, startLocation);
					}

					ast_T* functionCall =
						parserParseFunctionCallNamed(parser, scope, qualifiedName);

					free(qualifiedName);
					return functionCall;
				}

				bool isBuiltinNamespace = strcmp(qualifiedName, "std:array") == 0;

				if (scopeGetQualifiedConstructDefinition(scope, qualifiedName) == (void*)0 && !isBuiltinNamespace) {
					ast_T* variable = astInit(AST_VARIABLE);
					variable->variableName = qualifiedName;
					variable->scope = scope;
					variable->location = startLocation;

					return variable;
				}

				char* constructName = qualifiedName;

				parserEat(parser, TOKEN_DOT);

				if (parser->currentToken->type != TOKEN_ID) {
					parserReportError(parser, "E1120", "Expected a method name after a qualified construct.");
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
                parserSetNodeLocation(object, parser->currentToken);
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
                    parserReportError(parser, "E1121", "Expected field or method name after '.'.");
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
                        parserReportError(parser, "E1122", "Expected a construct name before method call.");
                        free(fieldName);
                        astFree(object);
                        exit(EXIT_FAILURE);
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
                            ast_T* argument = parserParseComparison(parser, scope);

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
			
			object->location = startLocation;
            return object;
        }

        case TOKEN_LEFTPAREN: {
            parserEat(parser, TOKEN_LEFTPAREN);

            ast_T* expression = parserParseComparison(parser, scope);

            parserEat(parser, TOKEN_RIGHTPAREN);
            return expression;
        }

        default: {
			char message[256];

			const char* tokenValue = parser->currentToken->value;
			const char* tokenName = parserTokenTypeName(parser->currentToken->type);

			snprintf(
				message,
				sizeof(message),
				"Expected an expression, but found %s%s%s.",
				tokenValue != (void*)0 ? "'" : "",
				tokenValue != (void*)0 ? tokenValue : tokenName,
				tokenValue != (void*)0 ? "'" : ""
			);

			parserReportError(parser, "E1102", message);
			exit(EXIT_FAILURE);
		}
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
		strcmp(parser->currentToken->value, "float") == 0 ||
		strcmp(parser->currentToken->value, "array") == 0
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
