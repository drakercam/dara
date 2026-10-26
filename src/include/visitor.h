#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "value.h"
#include <stdbool.h>

typedef struct VISITOR_STRUCT {
	bool shouldReturn;
	ast_T* currentMethod;
	ast_T* currentConstruct;
	
	const char* fileName;
} visitor_T;

visitor_T* visitorInit();
void visitorFree(visitor_T* visitor);

value_T* visitorVisit(visitor_T* visitor, ast_T* node);

value_T* visitorVisitIfStatement(visitor_T* visitor, ast_T* node);
value_T* visitorVisitWhileLoop(visitor_T* visitor, ast_T* node);
value_T* visitorVisitForLoop(visitor_T* visitor, ast_T* node);
bool visitorArgumentMatchesType(ast_T* parameter, value_T* argument);
void visitorReportDiagnostic(visitor_T* visitor, ast_T* node, const char* code, const char* message);

/*
 * Evaluates a string literal.
 */
value_T* visitorVisitString(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a numeric literal.
 */
value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a boolean literal.
 */
value_T* visitorVisitBoolean(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a compound of statements.
 */
value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a binary arithmetic or comparison operation.
 */
value_T* visitorVisitBinaryOperation(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a unary operation.
 */
value_T* visitorVisitUnaryOperation(visitor_T* visitor, ast_T* node);

/*
 * Recursively creates a construct with default values for its fields.
 * Detects cyclic field definitions and limits nesting depth.
 */

value_T* visitorCreateDefaultConstruct(visitor_T* visitor, scope_T* scope, ast_T* definition, const char** typeStack, size_t depth);
	
/*
 * Creates a construct instance, initializes its default fields, and
 * applies the explicitly provided field initializers.
 */

value_T* visitorVisitConstructInstantiation(visitor_T* visitor, ast_T* node);

/*
 * Creates an array with a specified capacity and element type.
 */
value_T* visitorVisitArrayConstructor(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a method call belonging to the std:array namespace.
 */
value_T* visitorVisitArrayMethodCall(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a variable definition and registers the variable.
 */
value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a variable lookup.
 */
value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node);

/*
 * Resolves a variable or field to its actual stored value.
 */
value_T* visitorGetAssignmentTargetValue(visitor_T* visitor, ast_T* target);

/*
 * Evaluates an assignment to a variable or construct field.
 */
value_T* visitorVisitAssignment(visitor_T* visitor, ast_T* node);

/*
 * Registers a function definition in its scope.
 */
value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node);

/*
 * Registers function definitions contained within a namespace.
 */
value_T* visitorVisitNamespaceDefinition(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a function call, binds arguments, and checks its return type.
 */
value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a method call on a construct and manages the current method
 * and construct context.
 */
value_T* visitorVisitMethodCall(visitor_T* visitor, ast_T* node);

/*
 * Evaluates a return statement and signals that execution should return.
 */
value_T* visitorVisitReturn(visitor_T* visitor, ast_T* node);

#endif
