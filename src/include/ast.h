#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>

#include "scope.h"

typedef struct AST_STRUCT {
	enum {
		AST_VARIABLE_DEFINITION,
		AST_FUNCTION_DEFINITION,
		AST_CONSTRUCT_DEFINITION,
		AST_NAMESPACE_DEFINITION,
		AST_VARIABLE,
		AST_FUNCTION_CALL,
		AST_METHOD_CALL,
		AST_STRING,
		AST_COMPOUND,
		AST_NOOP,
		AST_NUMBER,
		AST_BOOLEAN,
		AST_UNARY_OPERATION,
		AST_BINARY_OPERATION,
		AST_RETURN,
		AST_CONSTRUCT_INSTANTIATION,
		AST_MEMBER_ACCESS,
		AST_IF_STATEMENT,
		AST_WHILE_LOOP,
		AST_ASSIGNMENT,
		AST_FOR_LOOP,
		AST_SWITCH_STATEMENT,
		AST_ENUM_DEFINITION,
		
	} type;
	
	// scope contains function and variable definitions
	struct SCOPE_STRUCT* scope;
	
	// AST_VARIABLE_DEFINITION
	char* variableDefinitionType;
	char* variableDefinitionVariableName;
	struct AST_STRUCT* variableDefinitionValue;
	
	// AST_FUNCTION_DEFINITION
	struct AST_STRUCT* functionDefinitionBody;	// attach compound to the body
	char* functionDefinitionName;
	struct AST_STRUCT** functionDefinitionArgs;
	size_t functionDefinitionArgsSize;
	char* functionDefinitionReturnType;
	
	// AST_VARIABLE
	char* variableName;
	
	// AST_FUNCTION_CALL
	char* functionCallName;
	struct AST_STRUCT** functionCallArguments;
	size_t functionCallArgumentsSize;
	
	// AST_METHOD_CALL
	char* methodCallConstructName;
    char* methodCallName;
    struct AST_STRUCT** methodCallArguments;
    size_t methodCallArgumentsSize;
	
	// AST_STRING
	char* stringValue;
	
	// AST_COMPOUND
	struct AST_STRUCT** compoundValue;
	size_t compoundSize;
	
	// AST_NUMBER
	bool numberIsFloat;
	double numberValue;
	
	// AST_BOOLEAN
	bool booleanValue;
	
	// AST_UNARY_OPERATION
	struct AST_STRUCT* unaryOperationOperand;
	int unaryOperationType;
	
	// AST_BINARY_OPERATION
	struct AST_STRUCT* binaryOperationLeft;
	struct AST_STRUCT* binaryOperationRight;
	int binaryOperationType;
	
	// AST_RETURN
	struct AST_STRUCT* returnValue;
	
	// AST_CONSTRUCT_DEFINITION
	char* constructDefinitionName;
	char** constructDefinitionFields;
	size_t constructDefinitionFieldsSize;
	char** constructDefinitionFieldTypes;
	struct AST_STRUCT** constructDefinitionMethods;
	size_t constructDefinitionMethodsSize;
	
	// AST_NAMESPACE_DEFINITION
	char* namespaceDefinitionName;
	struct SCOPE_STRUCT* namespaceDefinitionScope;
	struct AST_STRUCT* namespaceDefinitionBody;
	
	// AST_CONSTRUCT_INSTANTIATION
	char* constructInstantiationName;
	char** constructInstantiationFieldNames;
	struct AST_STRUCT** constructInstantiationFieldValues;
	size_t constructInstantiationFieldsSize;
	
	// AST_MEMBER_ACCESS
	struct AST_STRUCT* memberAccessObject;
	char* memberAccessField;
	
	// AST_IF_STATEMENT
	struct AST_STRUCT* ifCondition;
	struct AST_STRUCT* ifBody;
	struct AST_STRUCT* elseBody;

	// AST_WHILE_LOOP
	struct AST_STRUCT* whileCondition;
	struct AST_STRUCT* whileBody;
	
	// AST_ASSIGNMENT
	struct AST_STRUCT* assignmentTarget;
	struct AST_STRUCT* assignmentValue;
	
	// AST_FOR_LOOP
	struct AST_STRUCT* forInitializer;
	struct AST_STRUCT* forCondition;
	struct AST_STRUCT* forUpdate;
	struct AST_STRUCT* forBody;
	scope_T* forScope;
	
} ast_T;

ast_T* astInit(int type);
void astFree(ast_T* ast);
void astFreeVariableDefinition(ast_T* ast);

#endif
