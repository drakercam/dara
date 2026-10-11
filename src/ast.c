#include "include/ast.h"

#include <stdlib.h>

ast_T* astInit(int type) {
	ast_T* ast = calloc(1, sizeof(ast_T));
	ast->type = type;
	
	ast->scope = (void*)0;
	
	// AST_VARIABLE_DEFINITION
	ast->variableDefinitionVariableName = (void*)0;
	ast->variableDefinitionValue = (void*)0;
	
	// AST_FUNCTION_DEFINITION
	ast->functionDefinitionBody = (void*)0;
	ast->functionDefinitionName = (void*)0;
	ast->functionDefinitionArgs = (void*)0;
	ast->functionDefinitionArgsSize = 0;
	
	// AST_VARIABLE
	ast->variableName = (void*)0;
	
	// AST_FUNCTION_CALL
	ast->functionCallName = (void*)0;
	ast->functionCallArguments = (void*)0;
	ast->functionCallArgumentsSize = 0;
	
	// AST_STRING
	ast->stringValue = (void*) 0;
	
	// AST_COMPOUND
	ast->compoundValue = (void*)0;
	ast->compoundSize = 0;
	
	// AST_RETURN
	ast->returnValue = (void*)0;
	
	return ast;
}

void astFree(ast_T* ast)
{
    if (ast == NULL)
        return;

    switch (ast->type) {
        case AST_VARIABLE_DEFINITION:
            free(ast->variableDefinitionVariableName);
            astFree(ast->variableDefinitionValue);
            break;

        case AST_FUNCTION_DEFINITION:
            free(ast->functionDefinitionName);

            for (size_t i = 0; i < ast->functionDefinitionArgsSize; ++i)
                astFree(ast->functionDefinitionArgs[i]);

            free(ast->functionDefinitionArgs);

            astFree(ast->functionDefinitionBody);
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
