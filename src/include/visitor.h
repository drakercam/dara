#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"

typedef struct VISITOR_STRUCT {
	
} visitor_T;

visitor_T* visitorInit();
void visitorFree(visitor_T* visitor);

ast_T* visitorVisit(visitor_T* visitor, ast_T* node);

ast_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node);
ast_T* visitorVisitFunctionDefinition(visitor_T* visitor, ast_T* node);
ast_T* visitorVisitVariable(visitor_T* visitor, ast_T* node);
ast_T* visitorVisitFunctionCall(visitor_T* visitor, ast_T* node);
ast_T* visitorVisitString(visitor_T* visitor, ast_T* node);
ast_T* visitorVisitCompound(visitor_T* visitor, ast_T* node);

#endif
