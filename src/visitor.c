#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static value_T* visitorGetAssignmentTargetValue(
    visitor_T* visitor,
    ast_T* target
);

static value_T* visitorVisitMethodCall(
    visitor_T* visitor,
    ast_T* node
);

static value_T* builtinFunctionPrint(visitor_T* visitor, ast_T** args, int argsSize) {
	for (int i = 0; i < argsSize; ++i) {
		value_T* value = visitorVisit(visitor, args[i]);
		
		if (value == (void*)0) {
			continue;
		}
		
		switch (value->type) {
			
			case VALUE_NULL:
				printf("noval\n");
				break;
				
			case VALUE_INT:
				printf("%d\n", value->intValue);
				break;

			case VALUE_FLOAT:
				printf("%g\n", value->floatValue);
				break;
				
			case VALUE_BOOLEAN:
				printf("%s\n", value->booleanValue ? "true" : "false");
				break;
			
			case VALUE_STRING:
				printf("%s\n", value->stringValue);
				break;
				
			case VALUE_CONSTRUCT:
				printf("<construct>\n");
				break;
		}
		
		valueFree(value);
	}
	
	return (void*)0;
}


static value_T* builtinFunctionAdd(visitor_T* visitor, ast_T** args, int argsSize) {
    if (argsSize == 0) {
        return valueInitInt(0);
    }

    value_T* first = visitorVisit(visitor, args[0]);

    if (first == NULL) {
        return NULL;
    }

    if (first->type != VALUE_INT && first->type != VALUE_FLOAT) {
        printf("ERROR::BUILTIN::ADD: Expected numeric arguments\n");
        valueFree(first);
        exit(1);
    }

    int resultInt = 0;
    double resultFloat = 0.0;
    int resultType = first->type;

    if (resultType == VALUE_INT) {
        resultInt = first->intValue;
    } else {
        resultFloat = first->floatValue;
    }

    valueFree(first);

    for (int i = 1; i < argsSize; ++i) {
        value_T* value = visitorVisit(visitor, args[i]);

        if (value == NULL) {
            continue;
        }

        if (value->type != resultType) {
            printf("ERROR::BUILTIN::ADD: Arguments must have matching numeric types\n");
            valueFree(value);
            exit(1);
        }

        if (resultType == VALUE_INT) {
            resultInt += value->intValue;
        } else {
            resultFloat += value->floatValue;
        }

        valueFree(value);
    }

    if (resultType == VALUE_INT) {
        return valueInitInt(resultInt);
    }

    return valueInitFloat(resultFloat);
}

static value_T* builtinFunctionSub(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

static value_T* builtinFunctionMultiply(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

static value_T* builtinFunctionDivide(visitor_T* visitor, ast_T** args, int argsSize) {
	
}

visitor_T* visitorInit() {
	visitor_T* visitor = calloc(1, sizeof(visitor_T));
	return visitor;
}

void visitorFree(visitor_T* visitor) {
	if (visitor == (void*)0) {
		return;
	}
	
	free(visitor);
}

value_T* visitorVisit(visitor_T* visitor, ast_T* node) {
	
	//printf("type=%d\n", node->type);
		
	switch(node->type) {
		
		case AST_VARIABLE_DEFINITION:
			return visitorVisitVariableDefinition(visitor, node);
			break;
		
		case AST_FUNCTION_DEFINITION:
			return visitorVisitFunctionDefinition(visitor, node);
			break;
			
		case AST_CONSTRUCT_DEFINITION:
			return (void*)0;
			break;
			
		case AST_NAMESPACE_DEFINITION:
			return visitorVisitNamespaceDefinition(visitor, node);
			break;
			
		case AST_CONSTRUCT_INSTANTIATION:
			return visitorVisitConstructInstantiation(visitor, node);
			break;
		
		case AST_VARIABLE:
			return visitorVisitVariable(visitor, node);
			break;
			
		case AST_FUNCTION_CALL:
			return visitorVisitFunctionCall(visitor, node);
			break;
			
		case AST_METHOD_CALL:
			return visitorVisitMethodCall(visitor, node);
			
		case AST_STRING:
			return visitorVisitString(visitor, node);
			break;
			
		case AST_NUMBER:
			return visitorVisitNumber(visitor, node);
			break;
			
		case AST_BOOLEAN:
			return visitorVisitBoolean(visitor, node);
			break;
			
		case AST_COMPOUND:
			return visitorVisitCompound(visitor, node);
			break;
			
		case AST_IF_STATEMENT:
			return visitorVisitIfStatement(visitor, node);
			break;
			
		case AST_WHILE_LOOP:
			return visitorVisitWhileLoop(visitor, node);
			break;
			
		case AST_FOR_LOOP:
			return visitorVisitForLoop(visitor, node);
			break;
			
		case AST_ASSIGNMENT:
			return visitorVisitAssignment(visitor, node);
			break;
			
		case AST_BINARY_OPERATION:
			return visitorVisitBinaryOperation(visitor, node);
			break;
			
		case AST_UNARY_OPERATION:
			return visitorVisitUnaryOperation(visitor, node);
			break;
			
		case AST_RETURN:
			return visitorVisitReturn(visitor, node);
			break;
			
		case AST_MEMBER_ACCESS: {
			value_T* object = visitorVisit(
				visitor,
				node->memberAccessObject
			);

			if (object == NULL || object->type != VALUE_CONSTRUCT) {
				printf("Cannot access a field on a non-construct value\n");
				valueFree(object);
				exit(1);
			}

			value_T* field = valueConstructGetField(
				object,
				node->memberAccessField
			);

			if (field == NULL) {
				printf(
					"Construct has no field named '%s'\n",
					node->memberAccessField
				);
				valueFree(object);
				exit(1);
			}

			value_T* result = valueCopy(field);

			valueFree(object);

			return result;
		}
		
		case AST_NOOP:
			return (void*)0;
			
	}
	
	printf("Uncaught statement of type '%d'\n", node->type);
	return (void*)0;
}

value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node) {
	value_T* value = visitorVisit(visitor, node->variableDefinitionValue);
	
    if (value == (void*)0) {
        printf("Failed to evaluate initializer for variable '%s'\n", node->variableDefinitionVariableName);
        exit(1);
    }

    char* declaredType = node->variableDefinitionType;
    if (strcmp(declaredType, "int") == 0 && value->type != VALUE_INT) {
        printf("Type error: variable '%s' expects int\n", node->variableDefinitionVariableName);
        valueFree(value);
        exit(1);
    }

    if (strcmp(declaredType, "float") == 0 && value->type != VALUE_FLOAT) {
        printf("Type error: variable '%s' expects float\n", node->variableDefinitionVariableName);
        valueFree(value);
        exit(1);
    }
    
    if (strcmp(declaredType, "string") == 0 &&
		value->type != VALUE_STRING) {
		printf("Type error: variable '%s' expects string\n",
			node->variableDefinitionVariableName);
		valueFree(value);
		exit(1);
	}

	bool isBuiltinType =
		strcmp(declaredType, "auto") == 0 ||
		strcmp(declaredType, "int") == 0 ||
		strcmp(declaredType, "float") == 0 ||
		strcmp(declaredType, "string") == 0;

	if (!isBuiltinType) {
		ast_T* constructDefinition =
			scopeGetQualifiedConstructDefinition(node->scope, declaredType);

		if (constructDefinition == (void*)0) {
			printf("Unknown type '%s'\n", declaredType);
			valueFree(value);
			exit(1);
		}

		if (value->type != VALUE_CONSTRUCT ||
			strcmp(value->constructValue.name, declaredType) != 0) {
			printf("Type error: variable '%s' expects construct '%s'\n",
				node->variableDefinitionVariableName,
				declaredType);
			valueFree(value);
			exit(1);
		}
	}

    scopeAddVariable(node->scope, node->variableDefinitionVariableName, value);

    return (void*)0;
}

value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node) {
	
	scopeAddFunctionDefinition(node->scope, node);
	
	return (void*)0;
}

value_T* visitorVisitNamespaceDefinition(visitor_T* visitor, ast_T* node) {
	// register the definitions inside the namespaces scope
	for (size_t i = 0; i < node->namespaceDefinitionBody->compoundSize; ++i) {
		ast_T* statement = node->namespaceDefinitionBody->compoundValue[i];
		
		if (statement->type == AST_FUNCTION_DEFINITION) {
			visitorVisitFunctionDefinition(visitor, statement);
		}
	}
	
	return (void*)0;
}

static value_T* visitorCreateDefaultConstruct(
    scope_T* scope,
    ast_T* definition,
    const char** typeStack,
    size_t depth
) {
    // Prevent infinite recursion from cyclic construct fields.
    if (depth >= 64) {
        printf("Construct nesting exceeds maximum depth\n");
        exit(1);
    }

    const char* name = definition->constructDefinitionName;

    for (size_t i = 0; i < depth; ++i) {
        if (strcmp(typeStack[i], name) == 0) {
            printf("Recursive construct field type '%s'\n", name);
            exit(1);
        }
    }

    typeStack[depth] = name;

    value_T* instance = valueInitConstruct(name);

    for (size_t i = 0;
         i < definition->constructDefinitionFieldsSize;
         ++i) {

        char* fieldName = definition->constructDefinitionFields[i];
        char* fieldType = definition->constructDefinitionFieldTypes[i];

        value_T* fieldValue = (void*)0;

        if (strcmp(fieldType, "int") == 0) {
            fieldValue = valueInitInt(0);
        }
        else if (strcmp(fieldType, "float") == 0) {
            fieldValue = valueInitFloat(0.0);
        }
        else if (strcmp(fieldType, "string") == 0) {
            fieldValue = valueInitString("");
        }
        else if (strcmp(fieldType, "auto") == 0) {
            fieldValue = valueInit(VALUE_NULL);
        }
        else {
            ast_T* nestedDefinition =
                scopeGetConstructDefinition(scope, fieldType);

            if (nestedDefinition == (void*)0) {
                printf("Unknown field type '%s'\n", fieldType);
                valueFree(instance);
                exit(1);
            }

            fieldValue = visitorCreateDefaultConstruct(
                scope,
                nestedDefinition,
                typeStack,
                depth + 1
            );
        }

        valueConstructSetField(instance, fieldName, fieldValue);
    }

    return instance;
}

value_T* visitorVisitConstructInstantiation(visitor_T* visitor, ast_T* node) {
    ast_T* definition = scopeGetQualifiedConstructDefinition(
        node->scope,
        node->constructInstantiationName
    );

    if (definition == (void*)0) {
        printf("Undefined construct %s\n", node->constructInstantiationName);
        exit(1);
    }

	const char* typeStack[64] = {0};

	value_T* instance = visitorCreateDefaultConstruct(
		node->scope,
		definition,
		typeStack,
		0
	);
	
	free(instance->constructValue.name);

	instance->constructValue.name = calloc(
		strlen(node->constructInstantiationName) + 1,
		sizeof(char)
	);

	if (instance->constructValue.name == (void*)0) {
		printf("Failed to allocate construct name\n");
		valueFree(instance);
		exit(EXIT_FAILURE);
	}

	strcpy(
		instance->constructValue.name,
		node->constructInstantiationName
	);

    // Apply the explicitly provided initializers.
    for (size_t i = 0; i < node->constructInstantiationFieldsSize; ++i) {
        char* fieldName =
            node->constructInstantiationFieldNames[i];

        ast_T* fieldExpression =
            node->constructInstantiationFieldValues[i];
            
            // Reject duplicate initializers.
			for (size_t j = 0; j < i; ++j) {
				if (strcmp(
						node->constructInstantiationFieldNames[j],
						fieldName
					) == 0) {

					printf(
						"Duplicate initializer for field '%s' in construct '%s'\n",
						fieldName,
						node->constructInstantiationName
					);

					valueFree(instance);
					exit(1);
				}
			}

        // Make sure the field exists and find its declared type.
        size_t fieldIndex = 0;

        while (fieldIndex < definition->constructDefinitionFieldsSize &&
               strcmp(
                   definition->constructDefinitionFields[fieldIndex],
                   fieldName
               ) != 0) {
            ++fieldIndex;
        }

        if (fieldIndex == definition->constructDefinitionFieldsSize) {
            printf(
                "Construct '%s' has no field named '%s'\n",
                node->constructInstantiationName,
                fieldName
            );
            valueFree(instance);
            exit(1);
        }

        char* declaredType =
            definition->constructDefinitionFieldTypes[fieldIndex];

        // Evaluate the initializer expression.
        value_T* fieldValue = visitorVisit(visitor, fieldExpression);

        if (fieldValue == (void*)0) {
            printf("Failed to evaluate initializer for field '%s'\n", fieldName);
            valueFree(instance);
            exit(1);
        }

        // Enforce the declared field type.
        if ((strcmp(declaredType, "int") == 0 &&
             fieldValue->type != VALUE_INT) ||
            (strcmp(declaredType, "float") == 0 &&
             fieldValue->type != VALUE_FLOAT) ||
            (strcmp(declaredType, "string") == 0 &&
             fieldValue->type != VALUE_STRING)) {

            printf(
                "Type mismatch for field '%s' in construct '%s'\n",
                fieldName,
                node->constructInstantiationName
            );

            valueFree(fieldValue);
            valueFree(instance);
            exit(1);
        }

        // valueConstructSetField takes ownership of fieldValue.
        valueConstructSetField(instance, fieldName, fieldValue);
    }

    return instance;
}

value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	variable_T* var = scopeGetVariable(node->scope, node->variableName);
				
	if (var != (void*)0) {
		return valueCopy(var->value);
	}
	
	printf("Undefined variable '%s'\n", node->variableName);
	exit(1);
}

static bool visitorArgumentMatchesType(
    ast_T* parameter,
    value_T* argument
) {
    char* type = parameter->variableDefinitionType;

    if (strcmp(type, "auto") == 0) {
        return true;
    }

    if (strcmp(type, "int") == 0) {
        return argument->type == VALUE_INT;
    }

    if (strcmp(type, "float") == 0) {
        return argument->type == VALUE_FLOAT;
    }

    if (strcmp(type, "string") == 0) {
        return argument->type == VALUE_STRING;
    }

    // A non-builtin type should match a construct's name.
    if (argument->type == VALUE_CONSTRUCT) {
        return strcmp(
            type,
            argument->constructValue.name
        ) == 0;
    }

    return false;
}

value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->functionCallName, "print") == 0) {
		return builtinFunctionPrint(
			visitor,
			node->functionCallArguments,
			node->functionCallArgumentsSize
		);
	}
	
	if (strcmp(node->functionCallName, "add") == 0) {
		return builtinFunctionAdd(visitor, node->functionCallArguments, node->functionCallArgumentsSize);
	}
	
	ast_T* funcDef = scopeGetFunctionDefinition(
		node->scope,
		node->functionCallName
	);

	if (funcDef == (void*)0) {
		printf("Undefined function '%s'\n", node->functionCallName);
		exit(1);
	}

	if (funcDef->functionDefinitionArgsSize != node->functionCallArgumentsSize) {
		printf(
			"ERROR: Calling function '%s' with %zu argument(s)\n"
			"-- Expects %zu argument(s)\n",
			funcDef->functionDefinitionName,
			node->functionCallArgumentsSize,
			funcDef->functionDefinitionArgsSize
		);
		exit(1);
	}
	
	
	for (size_t i = 0; i < funcDef->functionDefinitionArgsSize; i++) {
		ast_T* parameter = funcDef->functionDefinitionArgs[i];
		ast_T* argumentNode = node->functionCallArguments[i];

		bool isReference = parameter->variableDefinitionIsReference;
		value_T* argument;

		if (isReference) {
			if (argumentNode->type != AST_VARIABLE &&
				argumentNode->type != AST_MEMBER_ACCESS) {
				printf(
					"Reference parameter '%s' requires a variable or field argument\n",
					parameter->variableDefinitionVariableName
				);
				exit(1);
			}

			argument = visitorGetAssignmentTargetValue(
				visitor,
				argumentNode
			);
		} else {
			argument = visitorVisit(visitor, argumentNode);
		}

		if (argument == (void*)0) {
			printf("Failed to evaluate function argument\n");
			exit(1);
		}

		if (!visitorArgumentMatchesType(parameter, argument)) {
			printf(
				"Argument type mismatch for parameter '%s'\n",
				parameter->variableDefinitionVariableName
			);

			if (!isReference) {
				valueFree(argument);
			}

			exit(1);
		}

		variable_T* binding;

		if (isReference) {
			binding = scopeAddReferenceVariable(
				funcDef->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		} else {
			binding = scopeAddVariable(
				funcDef->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		}

		if (binding == (void*)0) {
			if (!isReference) {
				valueFree(argument);
			}

			printf("Failed to bind function parameter\n");
			exit(1);
		}
	}

	visitor->shouldReturn = false;

	value_T* result = visitorVisit(visitor, funcDef->functionDefinitionBody);

	bool didReturn = visitor->shouldReturn;
	visitor->shouldReturn = false;


	if (!didReturn) {
		if (result != NULL) {
			valueFree(result);
		}

		result = valueInit(VALUE_NULL);
	}

	// Enforce the function's declared return type.
	char* returnType = funcDef->functionDefinitionReturnType;

	if (strcmp(returnType, "noval") == 0) {
		if (result->type != VALUE_NULL) {
			valueFree(result);
			printf(
				"Return type error: function '%s' declares noval\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "int") == 0) {
		if (!didReturn || result->type != VALUE_INT) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects int\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "float") == 0) {
		if (!didReturn || result->type != VALUE_FLOAT) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects float\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}
	else if (strcmp(returnType, "string") == 0) {
		if (!didReturn || result->type != VALUE_STRING) {
			valueFree(result);
			printf(
				"Return type error: function '%s' expects string\n",
				funcDef->functionDefinitionName
			);
			exit(1);
		}
	}

	scopeRemoveVariables(
		funcDef->functionDefinitionBody->scope,
		funcDef->functionDefinitionArgsSize
	);
	
	return result;
}


static value_T* visitorVisitMethodCall(visitor_T* visitor, ast_T* node) {
    // 1. Find the construct definition.
    ast_T* construct = scopeGetQualifiedConstructDefinition(
        node->scope,
        node->methodCallConstructName
    );

    if (construct == (void*)0) {
        printf(
            "Undefined construct '%s'\n",
            node->methodCallConstructName
        );
        exit(1);
    }

    // 2. Find the requested method.
    ast_T* method = (void*)0;

    for (size_t i = 0;
         i < construct->constructDefinitionMethodsSize;
         ++i) {

        ast_T* candidate = construct->constructDefinitionMethods[i];

        if (strcmp(
                candidate->functionDefinitionName,
                node->methodCallName
            ) == 0) {
            method = candidate;
            break;
        }
    }

    if (method == (void*)0) {
        printf(
            "Construct '%s' has no method named '%s'\n",
            node->methodCallConstructName,
            node->methodCallName
        );
        exit(1);
    }

    // 3. Check the number of arguments.
    if (method->functionDefinitionArgsSize != node->methodCallArgumentsSize) {
        printf(
            "Method '%s.%s' expects %zu argument(s), but received %zu\n",
            node->methodCallConstructName,
            node->methodCallName,
            method->functionDefinitionArgsSize,
            node->methodCallArgumentsSize
        );
        exit(1);
    }

    // 4. Evaluate arguments and bind them to parameters.
	for (size_t i = 0; i < method->functionDefinitionArgsSize; i++) {
		ast_T* parameter = method->functionDefinitionArgs[i];
		ast_T* argumentNode = node->methodCallArguments[i];

		bool isReference = parameter->variableDefinitionIsReference;
		value_T* argument;

		if (isReference) {
			if (argumentNode->type != AST_VARIABLE &&
				argumentNode->type != AST_MEMBER_ACCESS) {
				printf(
					"Reference parameter '%s' requires a variable or field argument\n",
					parameter->variableDefinitionVariableName
				);
				exit(1);
			}

			argument = visitorGetAssignmentTargetValue(
				visitor,
				argumentNode
			);
		} else {
			argument = visitorVisit(visitor, argumentNode);
		}

		if (argument == (void*)0) {
			printf("Failed to evaluate method argument\n");
			exit(1);
		}

		if (!visitorArgumentMatchesType(parameter, argument)) {
			printf(
				"Argument type mismatch for parameter '%s'\n",
				parameter->variableDefinitionVariableName
			);

			if (!isReference) {
				valueFree(argument);
			}

			exit(1);
		}

		variable_T* binding;

		if (isReference) {
			binding = scopeAddReferenceVariable(
				method->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		} else {
			binding = scopeAddVariable(
				method->functionDefinitionBody->scope,
				parameter->variableDefinitionVariableName,
				argument
			);
		}

		if (binding == (void*)0) {
			if (!isReference) {
				valueFree(argument);
			}

			printf("Failed to bind method parameter\n");
			exit(1);
		}
	}
    
	ast_T* previousMethod = visitor->currentMethod;
	ast_T* previousConstruct = visitor->currentConstruct;

	visitor->currentMethod = method;
	visitor->currentConstruct = construct;

    // 5. Execute the method body.
    visitor->shouldReturn = false;

    value_T* result = visitorVisit(
        visitor,
        method->functionDefinitionBody
    );

    bool didReturn = visitor->shouldReturn;
    visitor->shouldReturn = false;

    // Match ordinary function behavior when no return was encountered.
    if (!didReturn) {
        valueFree(result);
        result = valueInit(VALUE_NULL);
    }
    
    visitor->currentMethod = previousMethod;
	visitor->currentConstruct = previousConstruct;

    // 6. Enforce the declared return type.
    char* returnType = method->functionDefinitionReturnType;

    bool validReturn =
        (strcmp(returnType, "noval") == 0 &&
         result->type == VALUE_NULL) ||
        (strcmp(returnType, "int") == 0 &&
         didReturn && result->type == VALUE_INT) ||
        (strcmp(returnType, "float") == 0 &&
         didReturn && result->type == VALUE_FLOAT) ||
        (strcmp(returnType, "string") == 0 &&
         didReturn && result->type == VALUE_STRING);

    if (!validReturn) {
        printf(
            "Return type error: method '%s.%s' declares '%s'\n",
            node->methodCallConstructName,
            node->methodCallName,
            returnType
        );

        valueFree(result);
        exit(1);
    }

    // 7. Remove the parameter variables from the method scope.
    scopeRemoveVariables(
        method->functionDefinitionBody->scope,
        method->functionDefinitionArgsSize
    );

    return result;
}

value_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	return valueInitString(node->stringValue);
}

value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node) {
	if (node->numberIsFloat) {
		return valueInitFloat(node->numberValue);
	}

	return valueInitInt((int)node->numberValue);
}

value_T* visitorVisitBoolean(visitor_T* visitor, ast_T* node) {
	return valueInitBoolean(node->booleanValue);
}

value_T* visitorVisitIfStatement(visitor_T* visitor, ast_T* node) {
    value_T* condition = visitorVisit(visitor, node->ifCondition);

    if (condition == (void*)0) {
        printf("Failed to evaluate if condition\n");
        exit(1);
    }

    if (condition->type != VALUE_BOOLEAN) {
        printf("If condition must evaluate to a boolean\n");
        valueFree(condition);
        exit(1);
    }

    bool result = condition->booleanValue;
    valueFree(condition);

    if (result) {
        return visitorVisit(visitor, node->ifBody);
    }

    if (node->elseBody != NULL) {
        return visitorVisit(visitor, node->elseBody);
    }

    return valueInit(VALUE_NULL);
}


value_T* visitorVisitWhileLoop(visitor_T* visitor, ast_T* node) {
    value_T* result = valueInit(VALUE_NULL);

    while (true) {
        value_T* condition = visitorVisit(
            visitor,
            node->whileCondition
        );

        if (condition == (void*)0) {
            valueFree(result);
            printf("Failed to evaluate while condition\n");
            exit(1);
        }

        if (condition->type != VALUE_BOOLEAN) {
            valueFree(condition);
            valueFree(result);
            printf("While condition must evaluate to a boolean\n");
            exit(1);
        }

        bool shouldContinue = condition->booleanValue;
        valueFree(condition);

        if (!shouldContinue) {
            break;
        }

        value_T* bodyResult = visitorVisit(
            visitor,
            node->whileBody
        );

        // Preserve the most recent result without leaking the old one.
        valueFree(result);
        result = bodyResult;

        // A return inside the loop must exit the loop too.
        if (visitor->shouldReturn) {
            break;
        }
    }

    return result;
}

value_T* visitorVisitForLoop(visitor_T* visitor, ast_T* node) {
    scope_T* loopScope = node->forInitializer->scope;

    value_T* result = visitorVisit(
        visitor,
        node->forInitializer
    );
    valueFree(result);

    result = valueInit(VALUE_NULL);

    while (true) {
        value_T* condition = visitorVisit(
            visitor,
            node->forCondition
        );

        if (condition == (void*)0) {
            valueFree(result);
            printf("Failed to evaluate for condition\n");
            exit(1);
        }

        if (condition->type != VALUE_BOOLEAN) {
            valueFree(condition);
            valueFree(result);
            printf("For condition must evaluate to a boolean\n");
            exit(1);
        }

        bool shouldContinue = condition->booleanValue;
        valueFree(condition);

        if (!shouldContinue) {
            break;
        }

        value_T* bodyResult = visitorVisit(
            visitor,
            node->forBody
        );

        valueFree(result);
        result = bodyResult;

        if (visitor->shouldReturn) {
            break;
        }

        value_T* updateResult = visitorVisit(
            visitor,
            node->forUpdate
        );
        valueFree(updateResult);
    }

    // Remove the loop-local variables and free the loop scope.
    scopeRemoveVariables(loopScope, loopScope->variablesSize);

    return result;
}

static value_T* visitorGetAssignmentTargetValue(visitor_T* visitor, ast_T* target) {
    if (target->type == AST_VARIABLE) {
        variable_T* variable = scopeGetVariable(
            target->scope,
            target->variableName
        );

        if (variable == (void*)0) {
            printf(
                "Cannot assign to undefined variable '%s'\n",
                target->variableName
            );
            exit(1);
        }

        return variable->value;
    }

    if (target->type == AST_MEMBER_ACCESS) {
        // Resolve the actual stored object, not a copy.
        value_T* object = visitorGetAssignmentTargetValue(
            visitor,
            target->memberAccessObject
        );

        if (object == (void*)0 || object->type != VALUE_CONSTRUCT) {
            printf("Cannot assign a field on a non-construct value\n");
            exit(1);
        }

        value_T* field = valueConstructGetField(
            object,
            target->memberAccessField
        );

        if (field == (void*)0) {
            printf(
                "Unknown field '%s'\n",
                target->memberAccessField
            );
            exit(1);
        }

        return field;
    }

    printf("Invalid assignment target\n");
    exit(1);
}


value_T* visitorVisitAssignment(visitor_T* visitor, ast_T* node) {
    ast_T* target = node->assignmentTarget;

    // Evaluate the new value before modifying anything.
    value_T* newValue = visitorVisit(
        visitor,
        node->assignmentValue
    );

    if (newValue == (void*)0) {
        printf("Failed to evaluate assignment value\n");
        exit(1);
    }

    // Find the current value for type checking.
    value_T* oldValue = visitorGetAssignmentTargetValue(
        visitor,
        target
    );

    if (
        oldValue != (void*)0 &&
        oldValue->type != VALUE_NULL &&
        oldValue->type != newValue->type
    ) {
        printf("Type error: cannot assign a different type\n");
        valueFree(newValue);
        exit(EXIT_FAILURE);
    }

    // A construct variable or field must retain its construct type.
    if (
        oldValue != (void*)0 &&
        oldValue->type == VALUE_CONSTRUCT &&
        strcmp(
            oldValue->constructValue.name,
            newValue->constructValue.name
        ) != 0
    ) {
        printf("Type error: cannot assign a different construct type\n");
        valueFree(newValue);
        exit(1);
    }
	if (target->type == AST_VARIABLE) {
		variable_T* variable = scopeGetVariable(
			target->scope,
			target->variableName
		);

		if (variable->ownsValue) {
			// Ordinary variable: replace its owned value.
			valueFree(variable->value);
			variable->value = newValue;
		} else {
			// Reference parameter: preserve the referenced address.
			valueReplace(variable->value, newValue);
		}
	}
    else {
        // Resolve the containing object in memory.
        value_T* object = visitorGetAssignmentTargetValue(
            visitor,
            target->memberAccessObject
        );

        // valueConstructSetField takes ownership of newValue.
        valueConstructSetField(
            object,
            target->memberAccessField,
            newValue
        );
    }

    return valueInit(VALUE_NULL);
}


value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node) {
	value_T* result = (void*)0;
	
	for (size_t i = 0; i < node->compoundSize; ++i) {
		
		if (visitor->shouldReturn) {
			break;	// if the visitor visited a return statement, it stops execution and returns
		}
		
		valueFree(result);
		result = visitorVisit(visitor, node->compoundValue[i]);
	}
	
	// returns the value of its last statement, which will be useful for 
	// implementing return/expression handling
	return result;
}

value_T* visitorVisitBinaryOperation(visitor_T* visitor, ast_T* node) {
    value_T* left = visitorVisit(visitor, node->binaryOperationLeft);
    value_T* right = visitorVisit(visitor, node->binaryOperationRight);

    if (left == NULL || right == NULL) {
        valueFree(left);
        valueFree(right);
        return NULL;
    }

    if (left->type != right->type ||
        (left->type != VALUE_INT && left->type != VALUE_FLOAT)) {
        printf("Arithmetic operations require matching numeric types\n");
        valueFree(left);
        valueFree(right);
        exit(1);
    }
    
    int isInt = left->type == VALUE_INT;
    int op = node->binaryOperationType;

    // Handle comparisons first. They always return bool.
    switch (op) {
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_NOT_EQUAL:
        case TOKEN_LESS:
        case TOKEN_MORE:
        case TOKEN_LESS_THAN_EQUAL:
        case TOKEN_MORE_THAN_EQUAL: {
            double a = isInt ? left->intValue : left->floatValue;
            double b = isInt ? right->intValue : right->floatValue;
            bool result;

            switch (op) {
                case TOKEN_EQUAL_EQUAL:
                    result = a == b;
                    break;
                case TOKEN_NOT_EQUAL:
                    result = a != b;
                    break;
                case TOKEN_LESS:
                    result = a < b;
                    break;
                case TOKEN_MORE:
                    result = a > b;
                    break;
                case TOKEN_LESS_THAN_EQUAL:
                    result = a <= b;
                    break;
                case TOKEN_MORE_THAN_EQUAL:
                    result = a >= b;
                    break;
                default:
                    result = false;
                    break;
            }

            valueFree(left);
            valueFree(right);

            return valueInitBoolean(result);
        }
    }

    // Integer arithmetic stays integer arithmetic.
    if (isInt) {
        int a = left->intValue;
        int b = right->intValue;
        int result;

        switch (op) {
            case TOKEN_PLUS:
                result = a + b;
                break;

            case TOKEN_MINUS:
                result = a - b;
                break;

            case TOKEN_MULTIPLY:
                result = a * b;
                break;

            case TOKEN_DIVIDE:
                if (b == 0) {
                    printf("Division by zero\n");
                    valueFree(left);
                    valueFree(right);
                    exit(1);
                }

                result = a / b;
                break;

            default:
                printf("Unknown arithmetic operator\n");
                valueFree(left);
                valueFree(right);
                exit(1);
        }

        valueFree(left);
        valueFree(right);

        return valueInitInt(result);
    }

    // Floating-point arithmetic stays floating-point.
    double a = left->floatValue;
    double b = right->floatValue;
    double result;

    switch (op) {
        case TOKEN_PLUS:
            result = a + b;
            break;

        case TOKEN_MINUS:
            result = a - b;
            break;

        case TOKEN_MULTIPLY:
            result = a * b;
            break;

        case TOKEN_DIVIDE:
            if (b == 0.0) {
                printf("Division by zero\n");
                valueFree(left);
                valueFree(right);
                exit(1);
            }

            result = a / b;
            break;

        default:
            printf("Unknown arithmetic operator\n");
            valueFree(left);
            valueFree(right);
            exit(1);
    }

    valueFree(left);
    valueFree(right);

    return valueInitFloat(result);
}

value_T* visitorVisitUnaryOperation(visitor_T* visitor, ast_T* node) {
	value_T* operand = visitorVisit(visitor, node->unaryOperationOperand);
	
	if (operand == (void*)0) {
		return (void*)0;
	}
	
	switch(node->unaryOperationType) {
		case TOKEN_NOT: {
			if (operand->type != VALUE_BOOLEAN) {
				printf("Logical NOT requires a boolean operand\n");
				valueFree(operand);
				exit(1);
			}
			
			bool result = !operand->booleanValue;
			valueFree(operand);
			return valueInitBoolean(result);
		}
			
		default:
			printf("Unknown Unary Operator\n");
			valueFree(operand);
			exit(1);
	}
}

value_T* visitorVisitReturn(visitor_T* visitor, ast_T* node) {
	value_T* result;
	
	if (node->returnValue == NULL) {
		result = valueInit(VALUE_NULL);
	}
	else {
		result = visitorVisit(visitor, node->returnValue);
	}
	
	visitor->shouldReturn = true;
	
	return result;
}


