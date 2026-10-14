#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Evaluates a variable definition, checks its declared type, and
 * registers the variable in its scope.
 */

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

/*
 * Retrieves a copy of the value associated with a variable.
 */

value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	variable_T* var = scopeGetVariable(node->scope, node->variableName);
				
	if (var != (void*)0) {
		return valueCopy(var->value);
	}
	
	printf("Undefined variable '%s'\n", node->variableName);
	exit(1);
}

/*
 * Resolves a variable or construct field to its actual stored value.
 * Used by assignment and reference parameters.
 */

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

/*
 * Evaluates an assignment, checks type compatibility, and updates
 * the target variable or construct field.
 */

value_T* visitorGetAssignmentTargetValue(visitor_T* visitor, ast_T* target) {
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
