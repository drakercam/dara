#include "include/parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static bool parserIsConstructType(scope_T* scope, const char* name) {
    //~ printf("Checking type: %s (known constructs: %zu)\n",
        //~ name, scope->constructDefinitionsSize);

    return scopeGetConstructDefinition(scope, name) != (void*)0;
}

static char* parserParseQualifiedConstructName(parser_T* parser, scope_T* scope) {
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

ast_T* parserParseIfStatement(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_IF_STATEMENT);
    ast->scope = scope;

    // Consume "if".
    parserEat(parser, TOKEN_ID);

    // Parse the condition: if (condition)
    parserEat(parser, TOKEN_LEFTPAREN);
    ast->ifCondition = parserParseComparison(parser, scope);
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Parse the true branch.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->ifBody = parserParseStatements(parser, scope);
    parserEat(parser, TOKEN_RIGHTBRACE);

    // Parse an optional else or else-if branch.
    if (
        parser->currentToken->type == TOKEN_ID &&
        strcmp(parser->currentToken->value, "else") == 0
    ) {
        parserEat(parser, TOKEN_ID);

        if (
            parser->currentToken->type == TOKEN_ID &&
            strcmp(parser->currentToken->value, "if") == 0
        ) {
            // An else-if is another if statement.
            ast->elseBody = parserParseIfStatement(parser, scope);
        }
        else {
            // A regular else block.
            parserEat(parser, TOKEN_LEFTBRACE);
            ast->elseBody = parserParseStatements(parser, scope);
            parserEat(parser, TOKEN_RIGHTBRACE);
        }
    }

    return ast;
}

ast_T* parserParseWhileLoop(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_WHILE_LOOP);
	ast->scope = scope;
	
	parserEat(parser, TOKEN_ID);	// consume while
	
	// Parse the condition: while (condition)
    parserEat(parser, TOKEN_LEFTPAREN);
    ast->whileCondition = parserParseComparison(parser, scope);
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Parse the while condition body.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->whileBody = parserParseStatements(parser, scope);
    parserEat(parser, TOKEN_RIGHTBRACE);
    
    return ast;
}

ast_T* parserParseForLoop(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_FOR_LOOP);
    ast->scope = scope;

    // Create a scope dedicated to this loop.
    scope_T* loopScope = scopeInitChild(scope);
	ast->forScope = loopScope;

    // Consume "for".
    parserEat(parser, TOKEN_ID);
    parserEat(parser, TOKEN_LEFTPAREN);

    // Initializer: int i = 0
    ast->forInitializer = parserParseVariableDefinition(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_SEMI);

    // Condition: i < 5
    ast->forCondition = parserParseComparison(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_SEMI);

    // Update: i = i + 1
    ast->forUpdate = parserParseAssignment(
        parser,
        loopScope
    );
    parserEat(parser, TOKEN_RIGHTPAREN);

    // Loop body.
    parserEat(parser, TOKEN_LEFTBRACE);
    ast->forBody = parserParseStatements(parser, loopScope);
    parserEat(parser, TOKEN_RIGHTBRACE);

    return ast;
}

ast_T* parserParseAssignment(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_ASSIGNMENT);
    ast->scope = scope;

    // Parse the complete left-hand side.
    ast->assignmentTarget = parserParseFactor(parser, scope);

    // Only variables and member accesses can be assignment targets.
    if (
        ast->assignmentTarget->type != AST_VARIABLE &&
        ast->assignmentTarget->type != AST_MEMBER_ACCESS
    ) {
        printf("Invalid assignment target\n");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    parserEat(parser, TOKEN_EQUALS);

    // Parse the right-hand expression.
    ast->assignmentValue = parserParseComparison(parser, scope);

    return ast;
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

ast_T* parserParseExpression(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseTerm(parser, scope);

	while (parser->currentToken->type == TOKEN_PLUS || parser->currentToken->type == TOKEN_MINUS) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseTerm(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
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
				char* constructName =
					parserParseQualifiedConstructName(parser, scope);

				// Qualified construct names must be followed by a method.
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

				if (methodCall->methodCallName == NULL) {
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

				// Parse arguments, just as we do for ordinary method calls.
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

						if (newArguments == NULL) {
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

                    if (methodCall->methodCallConstructName == NULL) {
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

// handles multiplication and division which have a higher precedence than adding/subtracting
ast_T* parserParseTerm(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseUnary(parser, scope);

	while (parser->currentToken->type == TOKEN_MULTIPLY || parser->currentToken->type == TOKEN_DIVIDE) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseUnary(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

ast_T* parserParseFunctionCall(parser_T* parser, scope_T* scope) {
	//printf("func name: %s\n", parser->previousToken->value);
	ast_T* functionCall = astInit(AST_FUNCTION_CALL);
	
	functionCall->functionCallName = calloc(strlen(parser->previousToken->value) + 1, sizeof(char));
	strcpy(functionCall->functionCallName, parser->previousToken->value);
	
	parserEat(parser, TOKEN_LEFTPAREN);
	functionCall->functionCallArguments = calloc(1, sizeof(ast_T*));
	
	if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
		ast_T* astExpression = parserParseComparison(parser, scope);
		functionCall->functionCallArguments[0] = astExpression;
		functionCall->functionCallArgumentsSize += 1;
		
		while (parser->currentToken->type == TOKEN_COMMA) {
			parserEat(parser, TOKEN_COMMA);
			
			ast_T* astExpression = parserParseComparison(parser, scope);
			functionCall->functionCallArgumentsSize += 1;
			functionCall->functionCallArguments = realloc(functionCall->functionCallArguments, 
											  functionCall->functionCallArgumentsSize * sizeof(ast_T*));
			functionCall->functionCallArguments[functionCall->functionCallArgumentsSize-1] = astExpression;
		}
	}
	
	parserEat(parser, TOKEN_RIGHTPAREN);
	
	functionCall->scope = scope;
	
	return functionCall;
}

ast_T* parserParseVariable(parser_T* parser, scope_T* scope) {
	
	char* tokenValue = parser->currentToken->value;
	parserEat(parser, TOKEN_ID); // var name or function call name
	
	if (parser->currentToken->type == TOKEN_LEFTPAREN) {
		return parserParseFunctionCall(parser, scope);
	}
		
	ast_T* astVariable = astInit(AST_VARIABLE);
	astVariable->variableName = calloc(strlen(tokenValue) + 1, sizeof(char));
	strcpy(astVariable->variableName, tokenValue);
	
	astVariable->scope = scope;
	
	return astVariable;
}

ast_T* parserParseVariableDefinition(parser_T* parser, scope_T* scope) {
	char* variableDefinitionType = (void*)0;

	if (
		strcmp(parser->currentToken->value, "var") == 0 ||
		strcmp(parser->currentToken->value, "auto") == 0 ||
		strcmp(parser->currentToken->value, "int") == 0 ||
		strcmp(parser->currentToken->value, "float") == 0 ||
		strcmp(parser->currentToken->value, "string") == 0
	) {
		const char* typeName = parser->currentToken->value;

		variableDefinitionType = calloc(
			strlen(typeName) + 1,
			sizeof(char)
		);

		if (variableDefinitionType == (void*)0) {
			printf("Failed to allocate variable type\n");
			exit(EXIT_FAILURE);
		}

		if (strcmp(typeName, "var") == 0) {
			strcpy(variableDefinitionType, "auto");
		} else {
			strcpy(variableDefinitionType, typeName);
		}

		parserEat(parser, TOKEN_ID);
	} else {
		variableDefinitionType =
			parserParseQualifiedConstructName(parser, scope);
	}
	char* variableName = parser->currentToken->value;
	char* variableDefinitionVariableName = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
	strcpy(variableDefinitionVariableName, variableName);
	
	parserEat(parser, TOKEN_ID); // var name
	parserEat(parser, TOKEN_EQUALS);
	ast_T* variableDefinitionValue = parserParseComparison(parser, scope);
	
	ast_T* variableDefinition = astInit(AST_VARIABLE_DEFINITION);
	variableDefinition->variableDefinitionType = variableDefinitionType;
	variableDefinition->variableDefinitionVariableName = variableDefinitionVariableName;
	variableDefinition->variableDefinitionValue = variableDefinitionValue;
	
	variableDefinition->scope = scope;
	
	return variableDefinition;	
}

ast_T* parserParseReturnStatement(parser_T* parser, scope_T* scope) {
	parserEat(parser, TOKEN_ID);	// return
	ast_T* returnStatement = astInit(AST_RETURN);
	
	returnStatement->scope = scope;
	
	if (parser->currentToken->type != TOKEN_SEMI &&
		parser->currentToken->type != TOKEN_RIGHTBRACE) {
		returnStatement->returnValue = parserParseComparison(parser, scope);
	}
	
	return returnStatement;
}

ast_T* parserParseFunctionDefinition(parser_T* parser, scope_T* scope) {
	ast_T* ast = astInit(AST_FUNCTION_DEFINITION);
	
	parserEat(parser, TOKEN_ID);	// func keyword
	
	char* functionName = parser->currentToken->value;
	ast->functionDefinitionName = calloc(strlen(functionName) + 1, sizeof(char));
	strcpy(ast->functionDefinitionName, functionName);
	
	parserEat(parser, TOKEN_ID);	// func name
	
	parserEat(parser, TOKEN_LEFTPAREN);

	// Parse function parameters.
	if (parser->currentToken->type != TOKEN_RIGHTPAREN) {
		while (1) {
			if (parser->currentToken->type != TOKEN_ID) {
				printf("Expected parameter type or name\n");
				astFree(ast);
				exit(1);
			}

			// The first identifier may be a type or an untyped parameter name.
			char* firstName = parser->currentToken->value;

			char* parameterType = (void*)0;
			char* parameterName = (void*)0;

			parserEat(parser, TOKEN_ID);

			if (parser->currentToken->type == TOKEN_ID) {
				// Typed parameter: int amount, Human h, etc.
				parameterType = calloc(
					strlen(firstName) + 1,
					sizeof(char)
				);
				strcpy(parameterType, firstName);

				parameterName = calloc(
					strlen(parser->currentToken->value) + 1,
					sizeof(char)
				);
				strcpy(parameterName, parser->currentToken->value);

				parserEat(parser, TOKEN_ID);
			}
			else {
				// Preserve compatibility with existing untyped parameters.
				parameterType = calloc(5, sizeof(char));
				strcpy(parameterType, "auto");

				parameterName = calloc(
					strlen(firstName) + 1,
					sizeof(char)
				);
				strcpy(parameterName, firstName);
			}

			if (parameterType == (void*)0 || parameterName == (void*)0) {
				printf("Failed to allocate function parameter\n");
				free(parameterType);
				free(parameterName);
				astFree(ast);
				exit(EXIT_FAILURE);
			}

			ast_T* parameter = astInit(AST_VARIABLE_DEFINITION);
			parameter->variableDefinitionType = parameterType;
			parameter->variableDefinitionVariableName = parameterName;
			parameter->variableDefinitionValue = (void*)0;
			parameter->scope = scope;

			size_t newSize = ast->functionDefinitionArgsSize + 1;

			ast_T** newArgs = realloc(
				ast->functionDefinitionArgs,
				newSize * sizeof(ast_T*)
			);

			if (newArgs == (void*)0) {
				printf("Failed to allocate function parameters\n");
				astFree(parameter);
				astFree(ast);
				exit(EXIT_FAILURE);
			}

			ast->functionDefinitionArgs = newArgs;
			ast->functionDefinitionArgs[newSize - 1] = parameter;
			ast->functionDefinitionArgsSize = newSize;

			if (parser->currentToken->type != TOKEN_COMMA) {
				break;
			}

			parserEat(parser, TOKEN_COMMA);
		}
	}

	
	parserEat(parser, TOKEN_RIGHTPAREN);

	// Parse the return type: -> int, -> float, -> string, -> noval
	parserEat(parser, TOKEN_ARROW);

	if (parser->currentToken->type != TOKEN_ID) {
		printf("Expected function return type after '->'\n");
		astFree(ast);
		exit(1);
	}

	char* returnType = parser->currentToken->value;

	if (strcmp(returnType, "int") != 0 &&
		strcmp(returnType, "float") != 0 &&
		strcmp(returnType, "string") != 0 &&
		strcmp(returnType, "noval") != 0) {

		printf("Unknown function return type '%s'\n", returnType);
		astFree(ast);
		exit(1);
	}

	ast->functionDefinitionReturnType = calloc(
		strlen(returnType) + 1,
		sizeof(char)
	);

	if (ast->functionDefinitionReturnType == (void*)0) {
		printf("Failed to allocate function return type\n");
		astFree(ast);
		exit(EXIT_FAILURE);
	}

	strcpy(ast->functionDefinitionReturnType, returnType);

	parserEat(parser, TOKEN_ID);
	
	parserEat(parser, TOKEN_LEFTBRACE);
	ast->functionDefinitionBody = parserParseStatements(parser, scope); // the function body is a compound
	parserEat(parser, TOKEN_RIGHTBRACE);
	
	ast->scope = scope;
	
	return ast;
}


ast_T* parserParseNamespaceDefinition(parser_T* parser, scope_T* scope) {
    ast_T* ast = astInit(AST_NAMESPACE_DEFINITION);
    ast->scope = scope;

    parserEat(parser, TOKEN_ID); // namespace keyword

    if (parser->currentToken->type != TOKEN_ID) {
        printf("Expected namespace name\n");
        astFree(ast);
        exit(1);
    }

    ast->namespaceDefinitionName = calloc(
        strlen(parser->currentToken->value) + 1,
        sizeof(char)
    );

    if (ast->namespaceDefinitionName == (void*)0) {
        printf("Failed to allocate namespace name\n");
        astFree(ast);
        exit(EXIT_FAILURE);
    }

    strcpy(
        ast->namespaceDefinitionName,
        parser->currentToken->value
    );

    parserEat(parser, TOKEN_ID); // namespace name
    parserEat(parser, TOKEN_LEFTBRACE);

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

ast_T* parserParseString(parser_T* parser, scope_T* scope) {
	
	ast_T* astString = astInit(AST_STRING);
	astString->stringValue = calloc(strlen(parser->currentToken->value) + 1, sizeof(char));
    strcpy(astString->stringValue, parser->currentToken->value);
	
	parserEat(parser, TOKEN_STRING);
	
	astString->scope = scope;
	
	return astString;
}

ast_T* parserParseNumber(parser_T* parser, scope_T* scope) {
	ast_T* astNumber = astInit(AST_NUMBER);
	astNumber->numberIsFloat = strchr(parser->currentToken->value, '.') != (void*)0;
	astNumber->numberValue = strtod(parser->currentToken->value, (void*)0);
	
	parserEat(parser, TOKEN_NUMBER);
	
	astNumber->scope = scope;
	
	return astNumber;
}

ast_T* parserParseComparison(parser_T* parser, scope_T* scope) {
	ast_T* left = parserParseExpression(parser, scope);

	while (parser->currentToken->type == TOKEN_EQUAL_EQUAL || 
		   parser->currentToken->type == TOKEN_LESS ||
		   parser->currentToken->type == TOKEN_MORE ||
		   parser->currentToken->type == TOKEN_LESS_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_MORE_THAN_EQUAL ||
		   parser->currentToken->type == TOKEN_NOT_EQUAL) {
		int operationType = parser->currentToken->type;
		parserEat(parser, operationType);

		ast_T* right = parserParseExpression(parser, scope);

		ast_T* operation = astInit(AST_BINARY_OPERATION);
		operation->binaryOperationLeft = left;
		operation->binaryOperationRight = right;
		operation->binaryOperationType = operationType;
		operation->scope = scope;

		left = operation;
	}

	return left;
}

ast_T* parserParseUnary(parser_T* parser, scope_T* scope) {
	if (parser->currentToken->type == TOKEN_NOT) {
		int operationType = parser->currentToken->type;
		parserEat(parser, TOKEN_NOT);
		
		ast_T* operand = parserParseUnary(parser, scope);
		
		ast_T* operation = astInit(AST_UNARY_OPERATION);
		operation->unaryOperationOperand = operand;
		operation->unaryOperationType = operationType;
		operation->scope = scope;
		
		return operation;
	}
	
	return parserParseFactor(parser, scope);
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
