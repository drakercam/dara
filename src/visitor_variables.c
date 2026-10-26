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
    
    if (strcmp(typeName, "array") == 0) {
		array_T* array = calloc(1, sizeof(array_T));

		if (!array) {
			return (void*)0;
		}

		array->capacity = 1;
		array->size = 0;
		array->elementType = VALUE_NULL;

		array->elements = calloc(array->capacity, sizeof(value_T*));
		array->elementTypeName = calloc(5, sizeof(char));

		if (!array->elements || !array->elementTypeName) {
			free(array->elements);
			free(array->elementTypeName);
			free(array);
			return (void*)0;
		}

		strcpy(array->elementTypeName, "auto");

		value_T* value = valueInit(VALUE_ARRAY);

		if (!value) {
			free(array->elements);
			free(array->elementTypeName);
			free(array);
			return (void*)0;
		}

		value->arrayValue = array;
		return value;
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
	
    if (!value) {
		return (void*)0;
    }

    char* declaredType = node->variableDefinitionType;
    bool typeMatches = true;
    
    if (strcmp(declaredType, "int") == 0) {
        typeMatches = value->type == VALUE_INT;
    }

    else if (strcmp(declaredType, "float") == 0) {
        typeMatches = value->type == VALUE_FLOAT;
    }
    
    else if (strcmp(declaredType, "string") == 0) {
		typeMatches = value->type == VALUE_STRING;
	}
	
	else if (strcmp(declaredType, "boolean") == 0) {
        typeMatches = value->type == VALUE_BOOLEAN;
    }
	
	else if (strcmp(declaredType, "array") == 0) {
		typeMatches = value->type == VALUE_ARRAY;
	}
	
	else if (strcmp(declaredType, "auto") != 0) {
		ast_T* constructDefinition = scopeGetQualifiedConstructDefinition(node->scope, declaredType);
		
		if (constructDefinition == (void*)0) {
			visitorReportDiagnostic(visitor, node, "E2021", "Unknown variable type.");
			valueFree(value);
			return (void*)0;
		}
		
		typeMatches = (value->type == VALUE_CONSTRUCT && strcmp(value->constructValue.name, declaredType) == 0);
	}
	
	if (!typeMatches) {
		visitorReportDiagnostic(visitor, node, "E2020", "Variable initializer does not match declared type.");
		valueFree(value);
		return (void*)0;
	}
	
	scopeAddVariable(node->scope, node->variableDefinitionVariableName, value);

    return (void*)0;
}

/*
 * Retrieves a copy of the value associated with a variable.
 */

value_T* visitorVisitVariable(visitor_T* visitor, ast_T* node) {
		
	variable_T* var = scopeGetVariable(node->scope, node->variableName);
				
	if (var) {
		return valueCopy(var->value);
	}
	
	value_T* constant = builtinRaylibGetConstant(node->variableName);
	
	if (constant) {
		return constant;
	}
	
	char message[256];

    snprintf(
        message,
        sizeof(message),
        "Undefined variable '%s'.",
        node->variableName
    );

    visitorReportDiagnostic(visitor, node, "E2002", message);
    return (void*)0;
}

/*
 * Resolves a variable or construct field to its actual stored value.
 * Used by assignment and reference parameters.
 */

value_T* visitorVisitAssignment(visitor_T* visitor, ast_T* node) {
    ast_T* target = node->assignmentTarget;


    // Evaluate the right-hand side first.
    value_T* newValue = visitorVisit(visitor, node->assignmentValue);

    if (!newValue) {
        return (void*)0;
    }
    
    value_T* oldValue = visitorGetAssignmentTargetValue(visitor, target);
    
    if (!oldValue) {
		valueFree(newValue);
		return (void*)0;
	}

    // Compound assignment combines the existing target value with
    // the evaluated right-hand value.
    if (node->assignmentOperator == TOKEN_PLUS_EQUAL || node->assignmentOperator == TOKEN_MINUS_EQUAL) {
        if (oldValue->type != newValue->type || (oldValue->type != VALUE_INT && oldValue->type != VALUE_FLOAT)) {
			visitorReportDiagnostic(visitor, node, "E2022", "Compound assignment requires matching numeric types.");
            valueFree(newValue);
			return (void*)0;
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
        
        if (!newValue) {
			visitorReportDiagnostic(visitor, node, "E2015", "Failed to allocated compound assignment result.");
			return (void*)0;
		}
    }

    if (oldValue->type != VALUE_NULL && oldValue->type != newValue->type) {
        visitorReportDiagnostic(visitor, node, "E2023", "Cannot assign a value of a different type.");
        valueFree(newValue);
        return (void*)0;
    }

    // A construct variable or field must retain its construct type.
    if (oldValue != (void*)0 && oldValue->type == VALUE_CONSTRUCT && strcmp(oldValue->constructValue.name, newValue->constructValue.name) != 0) {
        visitorReportDiagnostic(visitor, node, "E2024", "Cannot assign a value of a different construct type.");
        valueFree(newValue);
        return (void*)0;
    }
	if (target->type == AST_VARIABLE) {
		variable_T* variable = scopeGetVariable(target->scope, target->variableName);

		if (!variable) {
			valueFree(newValue);
			return (void*)0;
		}

		if (variable->ownsValue) {
			// Ordinary variable: replace its owned value.
			valueFree(variable->value);
			variable->value = newValue;
		} else {
			// Reference parameter: preserve the referenced address.
			valueReplace(variable->value, newValue);
		}
	}
    else if (target->type == AST_MEMBER_ACCESS) {
        // Resolve the containing object in memory.
        value_T* object = visitorGetAssignmentTargetValue(visitor, target->memberAccessObject);

		if (!object) {
			valueFree(newValue);
			return (void*)0;
		}

        // valueConstructSetField takes ownership of newValue.
        valueConstructSetField(
            object,
            target->memberAccessField,
            newValue
        );
    }
    else {
		visitorReportDiagnostic(visitor, node, "E2025", "Invalid assignment target.");
		valueFree(newValue);
		return (void*)0;
	}

    return valueInit(VALUE_NULL);
}

/*
 * Evaluates an assignment, checks type compatibility, and updates
 * the target variable or construct field.
 */

value_T* visitorGetAssignmentTargetValue(visitor_T* visitor, ast_T* target) {
	
	if (!target) {
		return (void*)0;
	}
	
    if (target->type == AST_VARIABLE) {
        variable_T* variable = scopeGetVariable(target->scope, target->variableName);

        if (!variable) {
			char message[256];
			
			snprintf(
				message,
				sizeof(message),
				"Undefined variable '%s'.",
				target->variableName
			);
			
			visitorReportDiagnostic(visitor, target, "E2002", message);
			
			return (void*)0;
		}

        return variable->value;
    }

    if (target->type == AST_MEMBER_ACCESS) {
        // Resolve the actual stored object, not a copy.
        value_T* object = visitorGetAssignmentTargetValue(visitor, target->memberAccessObject);

        if (!object) {
			return (void*)0;
		}
		
		if (object->type != VALUE_CONSTRUCT) {
			visitorReportDiagnostic(visitor, target, "E2009", "Cannot access a field on a value that is not a construct.");
			return (void*)0;
		}

        value_T* field = valueConstructGetField(
            object,
            target->memberAccessField
        );

        if (!field) {
            visitorReportDiagnostic(visitor, target, "E2010", "Construct does not contain the requested field.");
            return (void*)0;
        }

        return field;
    }

    visitorReportDiagnostic(visitor, target, "E2025", "Invalid target assignment.");
    return (void*)0;
}
