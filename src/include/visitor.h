#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "value.h"
#include <stdbool.h>

typedef struct VISITOR_STRUCT {
	bool shouldReturn;
} visitor_T;

visitor_T* visitorInit();
void visitorFree(visitor_T* visitor);

value_T* visitorVisit(visitor_T* visitor, ast_T* node);

value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node);
value_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node);
value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node);
value_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node);
value_T* visitorVisitString(visitor_T* visitor, ast_T* node);
value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node);
value_T* visitorVisitBoolean(visitor_T* visitor, ast_T* node);
value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node);
value_T* visitorVisitBinaryOperation(visitor_T* visitor, ast_T* node);
value_T* visitorVisitUnaryOperation(visitor_T* visitor, ast_T* node);
value_T* visitorVisitReturn(visitor_T* visitor, ast_T* node);

#endif
