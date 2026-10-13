#include "include/ast.h"
#include "include/scope.h"
#include <stdlib.h>

ast_T* astInit(int type) {
	ast_T* ast = calloc(1, sizeof(ast_T));
	ast->type = type;
	
	ast->scope = (void*)0;
	
	// AST_VARIABLE_DEFINITION
	ast->variableDefinitionType = (void*)0;
	ast->variableDefinitionVariableName = (void*)0;
	ast->variableDefinitionValue = (void*)0;
	
	// AST_FUNCTION_DEFINITION
	ast->functionDefinitionBody = (void*)0;
	ast->functionDefinitionName = (void*)0;
	ast->functionDefinitionArgs = (void*)0;
	ast->functionDefinitionArgsSize = 0;
	ast->functionDefinitionReturnType = (void*)0;
	
	// AST_CONSTRUCT_DEFINITION
	ast->constructDefinitionName = (void*)0;
	ast->constructDefinitionFields = (void*)0;
	ast->constructDefinitionFieldsSize = 0;
	ast->constructDefinitionFieldTypes = (void*)0;
	ast->constructDefinitionMethods = (void*)0;
	ast->constructDefinitionMethodsSize = 0;
	
	// AST_NAMESPACE_DEFINITION
	ast->namespaceDefinitionName = (void*)0;
	ast->namespaceDefinitionScope = (void*)0;
	ast->namespaceDefinitionBody = (void*)0;
	
	// AST_VARIABLE
	ast->variableName = (void*)0;
	
	// AST_FUNCTION_CALL
	ast->functionCallName = (void*)0;
	ast->functionCallArguments = (void*)0;
	ast->functionCallArgumentsSize = 0;
	
	// AST_METHOD_CALL
	ast->methodCallConstructName = (void*)0;
    ast->methodCallName = (void*)0;
    ast->methodCallArguments = (void*)0;
    ast->methodCallArgumentsSize = 0;
	
	// AST_STRING
	ast->stringValue = (void*) 0;
	
	// AST_COMPOUND
	ast->compoundValue = (void*)0;
	ast->compoundSize = 0;
	
	// AST_RETURN
	ast->returnValue = (void*)0;
	
	// AST_CONSTRUCT_INSTANTIATION
	ast->constructInstantiationName = (void*)0;
	ast->constructInstantiationFieldNames = (void*)0;
	ast->constructInstantiationFieldValues = (void*)0;
	ast->constructInstantiationFieldsSize = 0;
	
	// AST_MEMBER_ACCESS
	ast->memberAccessObject = (void*)0;
	ast->memberAccessField = (void*)0;
	
	// AST_IF_STATEMENT
	ast->ifCondition = (void*)0;
	ast->ifBody = (void*)0;
	ast->elseBody = (void*)0;
	
	// AST_WHILE_LOOP
	ast->whileCondition = (void*)0;
	ast->whileBody = (void*)0;
	
	// AST_ASSIGNMENT
	ast->assignmentTarget = (void*)0;
	ast->assignmentValue = (void*)0;
	
	// AST_FOR_LOOP
	ast->forInitializer = (void*)0;
	ast->forCondition = (void*)0;
	ast->forUpdate = (void*)0;
	ast->forBody = (void*)0;
	ast->forScope = (void*)0;
	
	return ast;
}

void astFree(ast_T* ast)
{
    if (ast == NULL)
        return;

    switch (ast->type) {
        case AST_VARIABLE_DEFINITION:
			free(ast->variableDefinitionType);
            free(ast->variableDefinitionVariableName);
            astFree(ast->variableDefinitionValue);
            break;

        case AST_FUNCTION_DEFINITION:
            free(ast->functionDefinitionName);

            for (size_t i = 0; i < ast->functionDefinitionArgsSize; ++i)
                astFree(ast->functionDefinitionArgs[i]);

            free(ast->functionDefinitionArgs);

            astFree(ast->functionDefinitionBody);
            free(ast->functionDefinitionReturnType);
            break;
            
        case AST_CONSTRUCT_DEFINITION:
			free(ast->constructDefinitionName);

			for (size_t i = 0; i < ast->constructDefinitionFieldsSize; ++i) {
				if (ast->constructDefinitionFields != NULL)
					free(ast->constructDefinitionFields[i]);

				if (ast->constructDefinitionFieldTypes != NULL)
					free(ast->constructDefinitionFieldTypes[i]);
			}
			
			for (size_t i = 0; i < ast->constructDefinitionMethodsSize; ++i) {
				astFree(ast->constructDefinitionMethods[i]);
			}

			free(ast->constructDefinitionMethods);
	
            free(ast->constructDefinitionFields);
			free(ast->constructDefinitionFieldTypes);
            
			break;
			
		// AST_NAMESPACE_DEFINITION
		case AST_NAMESPACE_DEFINITION:
			free(ast->namespaceDefinitionName);
			astFree(ast->namespaceDefinitionBody);
			scopeFree(ast->namespaceDefinitionScope);
			break;

        case AST_VARIABLE:
            free(ast->variableName);
            break;

        case AST_FUNCTION_CALL:
            free(ast->functionCallName);

            for (size_t i = 0; i < ast->functionCallArgumentsSize; ++i)
                astFree(ast->functionCallArguments[i]);

            free(ast->functionCallArguments);
            break;
            
		case AST_METHOD_CALL:
			for (size_t i = 0; i < ast->methodCallArgumentsSize; ++i) {
				astFree(ast->methodCallArguments[i]);
			}
			
			free(ast->methodCallArguments);
			free(ast->methodCallName);
			free(ast->methodCallConstructName);
			
			break;

        case AST_STRING:
            free(ast->stringValue);
            break;

		case AST_NUMBER:
			break;
		
		case AST_BOOLEAN:
			break;

        case AST_COMPOUND:
            for (size_t i = 0; i < ast->compoundSize; ++i)
                astFree(ast->compoundValue[i]);

            free(ast->compoundValue);
            break;

		case AST_BINARY_OPERATION:
			astFree(ast->binaryOperationLeft);
			astFree(ast->binaryOperationRight);
			break;
			
		case AST_UNARY_OPERATION:
			astFree(ast->unaryOperationOperand);
			break;
			
		case AST_RETURN:
			astFree(ast->returnValue);
			break;
			
		case AST_CONSTRUCT_INSTANTIATION:
			free(ast->constructInstantiationName);

			for (size_t i = 0;
				 i < ast->constructInstantiationFieldsSize;
				 ++i) {
				free(ast->constructInstantiationFieldNames[i]);
				astFree(ast->constructInstantiationFieldValues[i]);
			}

			free(ast->constructInstantiationFieldNames);
			free(ast->constructInstantiationFieldValues);
			break;
			
		case AST_MEMBER_ACCESS:
			astFree(ast->memberAccessObject);
			free(ast->memberAccessField);
			break;
			
		case AST_IF_STATEMENT:
			astFree(ast->ifCondition);
			astFree(ast->ifBody);
			astFree(ast->elseBody);
			break;
			
		case AST_WHILE_LOOP:
			astFree(ast->whileCondition);
			astFree(ast->whileBody);
			break;
			
		case AST_ASSIGNMENT:
			astFree(ast->assignmentTarget);
			astFree(ast->assignmentValue);
			break;
			
		case AST_FOR_LOOP:
			astFree(ast->forInitializer);
			astFree(ast->forCondition);
			astFree(ast->forUpdate);
			astFree(ast->forBody);
			scopeFree(ast->forScope);
			break;

        case AST_NOOP:
            break;
    }

    free(ast);
}

void astFreeVariableDefinition(ast_T* ast) {
	if (ast == (void*)0) {
		return;
	}
	
	free(ast->variableDefinitionVariableName);
	free(ast);
}
