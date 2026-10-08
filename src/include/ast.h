#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef struct AST_STRUCT {
	enum {
		AST_VARIABLE_DEFINITION,
		AST_FUNCTION_DEFINITION,
		AST_VARIABLE,
		AST_FUNCTION_CALL,
		AST_STRING,
		AST_COMPOUND,
		AST_NOOP,
		AST_NUMBER
		
	} type;
	
	// scope contains function and variable definitions
	struct SCOPE_STRUCT* scope;
	
	// AST_VARIABLE_DEFINITION
	char* variableDefinitionVariableName;
	struct AST_STRUCT* variableDefinitionValue;
	
	// AST_FUNCTION_DEFINITION
	struct AST_STRUCT* functionDefinitionBody;	// attach compound to the body
	char* functionDefinitionName;
	struct AST_STRUCT** functionDefinitionArgs;
	size_t functionDefinitionArgsSize;
	
	// AST_VARIABLE
	char* variableName;
	
	// AST_FUNCTION_CALL
	char* functionCallName;
	struct AST_STRUCT** functionCallArguments;
	size_t functionCallArgumentsSize;
	
	// AST_STRING
	char* stringValue;
	
	// AST_COMPOUND
	struct AST_STRUCT** compoundValue;
	size_t compoundSize;
	
	// AST_NUMBER
	double numberValue;
	
} ast_T;

ast_T* astInit(int type);
void astFree(ast_T* ast);
void astFreeVariableDefinition(ast_T* ast);

#endif
