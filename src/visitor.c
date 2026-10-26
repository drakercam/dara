#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/token.h"
#include "include/builtin_functions.h"
#include "include/diagnostic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
	
	if (!node) {
		return (void*)0;
	}
		
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
			
		case AST_ARRAY_CONSTRUCTOR:
			return visitorVisitArrayConstructor(visitor, node);
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
			value_T* object = visitorVisit(visitor, node->memberAccessObject);

			if (!object) {
				return (void*)0;
			}

			if (object->type != VALUE_CONSTRUCT) {
				visitorReportDiagnostic(visitor, node, "E2009", "Cannot access a field on a value that is not a construct.");
				valueFree(object);
				return (void*)0;
			}

			value_T* field = valueConstructGetField(object, node->memberAccessField);

			if (!field) {
				visitorReportDiagnostic(visitor, node, "E2010", "Construct does not contain the requested field.");
				valueFree(object);
				return (void*)0;
			}

			value_T* result = valueCopy(field);

			valueFree(object);

			return result;
		}
		
		case AST_NOOP:
			return (void*)0;
			
		default:
			visitorReportDiagnostic(visitor, node, "E2011", "Unsupported or unknown AST node type.");
			return (void*)0;
			
	}
	
	visitorReportDiagnostic(visitor, node, "E2011", "Unsupported or unknown AST node type.");
	return (void*)0;
}

bool visitorArgumentMatchesType(ast_T* parameter, value_T* argument) {
	if (!parameter || !argument || !parameter->variableDefinitionType) {
		return false;
	}
	
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

void visitorReportDiagnostic(visitor_T* visitor, ast_T* node, const char* code, const char* message) {
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
        DIAGNOSTIC_RUNTIME,
        ownedCode,
        ownedMessage,
        node->location
    );

    if (diagnostic == (void*)0) {
        free(ownedCode);
        free(ownedMessage);
        fprintf(stderr, "Failed to allocate diagnostic.\n");
        exit(EXIT_FAILURE);
    }

    diagnosticPrint(diagnostic, visitor->fileName);
    diagnosticFree(diagnostic);
}
