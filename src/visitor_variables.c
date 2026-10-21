#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"
#include "include/token.h"
#include "include/builtin_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/*
 * Creates the default value for a variable with no initializer.
 */
static value_T* visitorCreateDefaultValue(const char* typeName) {
    if (strcmp(typeName, "int") == 0) {
        return valueInitInt(0);
    }

    if (strcmp(typeName, "float") == 0) {
        return valueInitFloat(0.0);
    }

    if (strcmp(typeName, "string") == 0) {
        return valueInitString("");
    }

    return (void*)0;
}

/*
 * Evaluates a variable definition, checks its declared type, and
 * registers the variable in its scope.
 */

value_T* visitorVisitVariableDefinition(visitor_T* visitor, ast_T* node) {
	if (node->variableDefinitionValue == (void*)0) {
        if (strcmp(node->variableDefinitionType, "auto") == 0) {
            char message[256];

            snprintf(
                message,
                sizeof(message),
                "Cannot infer the type of variable '%s' without an initializer.",
                node->variableDefinitionVariableName
            );

            visitorReportDiagnostic(visitor, node, "E2001", message);
            exit(EXIT_FAILURE);
        }
    }
	
	value_T* value;

	if (node->variableDefinitionValue != (void*)0) {
		value = visitorVisit(visitor, node->variableDefinitionValue);
	} else {
		value = visitorCreateDefaultValue(node->variableDefinitionType);
	}
	
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
	
	value_T* constant = builtinRaylibGetConstant(node->variableName);
	
	if (constant != (void*)0) {
		return constant;
	}
	
	printf(
		"Runtime Error[E2002]: Undefined variable '%s'\n",
		node->variableName
	);
	exit(EXIT_FAILURE);
}

/*
 * Resolves a variable or construct field to its actual stored value.
 * Used by assignment and reference parameters.
 */

value_T* visitorVisitAssignment(visitor_T* visitor, ast_T* node) {
    ast_T* target = node->assignmentTarget;


    // Evaluate the right-hand side first.
    value_T* newValue = visitorVisit(visitor, node->assignmentValue);

    if (newValue == (void*)0) {
        printf("Failed to evaluate assignment value\n");
        exit(1);
    }

    // Compound assignment combines the existing target value with
    // the evaluated right-hand value.
    if (
        node->assignmentOperator == TOKEN_PLUS_EQUAL ||
        node->assignmentOperator == TOKEN_MINUS_EQUAL
    ) {
        value_T* oldValue = visitorGetAssignmentTargetValue(
            visitor,
            target
        );

        if (
            oldValue->type != newValue->type ||
            (oldValue->type != VALUE_INT &&
             oldValue->type != VALUE_FLOAT)
        ) {
            printf("Compound assignment requires matching numeric types\n");
            valueFree(newValue);
            exit(EXIT_FAILURE);
        }

        if (oldValue->type == VALUE_INT) {
            int result;

            if (node->assignmentOperator == TOKEN_PLUS_EQUAL) {
                result = oldValue->intValue + newValue->intValue;
            } else {
                result = oldValue->intValue - newValue->intValue;
            }

            valueFree(newValue);
            newValue = valueInitInt(result);
        } else {
            double result;

            if (node->assignmentOperator == TOKEN_PLUS_EQUAL) {
                result = oldValue->floatValue + newValue->floatValue;
            } else {
                result = oldValue->floatValue - newValue->floatValue;
            }

            valueFree(newValue);
            newValue = valueInitFloat(result);
        }
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
		variable_T* variable = scopeGetVariable(target->scope, target->variableName);

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
        variable_T* variable = scopeGetVariable(target->scope, target->variableName);

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
        value_T* object = visitorGetAssignmentTargetValue(visitor, target->memberAccessObject);

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
