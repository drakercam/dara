#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Recursively creates a construct with default values for its fields.
 * Detects cyclic field definitions and limits nesting depth.
 */

value_T* visitorCreateDefaultConstruct(visitor_T* visitor, scope_T* scope, ast_T* definition, const char** typeStack, size_t depth) {
    // Prevent infinite recursion from cyclic construct fields.
    if (depth >= 64) {
        visitorReportDiagnostic(visitor, definition, "E2012", "Construct nesting exceeds the maximum depth of 64.");
        return (void*)0;
    }

    const char* name = definition->constructDefinitionName;

    for (size_t i = 0; i < depth; ++i) {
        if (strcmp(typeStack[i], name) == 0) {
            visitorReportDiagnostic(visitor, definition, "E2013", "Recursive construct field type detected.");
            return (void*)0;
        }
    }

    typeStack[depth] = name;

    value_T* instance = valueInitConstruct(name);
    
    if (!instance) {
		visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocated construct instance.");
		return (void*)0;
	}

    for (size_t i = 0; i < definition->constructDefinitionFieldsSize; ++i) {

        char* fieldName = definition->constructDefinitionFields[i];
        char* fieldType = definition->constructDefinitionFieldTypes[i];

        value_T* fieldValue = (void*)0;
        bool nestedConstruct = false;

        if (strcmp(fieldType, "int") == 0) {
            fieldValue = valueInitInt(0);
        }
        else if (strcmp(fieldType, "float") == 0) {
            fieldValue = valueInitFloat(0.0);
        }
        else if (strcmp(fieldType, "string") == 0) {
            fieldValue = valueInitString("");
        }
        else if (strcmp(fieldType, "auto") == 0) {
            fieldValue = valueInit(VALUE_NULL);
        }
        else if (strcmp(fieldType, "array") == 0) {
			array_T* array = calloc(1, sizeof(array_T));
			
			if (!array) {
				visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocate default array field.");
				valueFree(instance);
				return (void*)0;
			}
			
			array->capacity = 1;
			array->size = 0;
			array->elementType = VALUE_NULL;
			
			array->elements = calloc(array->capacity, sizeof(value_T*));
			
			if (!array->elements) {
				visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocate default array elements.");
				free(array);
				valueFree(instance);
				return (void*)0;
			}
			
			array->elementTypeName = calloc(5, sizeof(char));
			
			if (!array->elementTypeName) {
				visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocate default array element type.");
				free(array->elements);
				free(array);
				valueFree(instance);
				return (void*)0;
			}
			
			strcpy(array->elementTypeName, "auto");
			
			fieldValue = valueInit(VALUE_ARRAY);
			
			if (!fieldValue) {
				free(array->elementTypeName);
				free(array->elements);
				free(array);
				
				visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocate default array value.");
				valueFree(instance);
				return (void*)0;
			}
			
			fieldValue->arrayValue = array;
		}
        else {
            ast_T* nestedDefinition =
                scopeGetConstructDefinition(scope, fieldType);

            if (!nestedDefinition) {
                visitorReportDiagnostic(visitor, definition, "E2014", "Unknown construct field type.");
                valueFree(instance);
                return (void*)0;
            }

            nestedConstruct = true;

            fieldValue = visitorCreateDefaultConstruct(
                visitor,
                scope,
                nestedDefinition,
                typeStack,
                depth + 1
            );
        }

        if (!fieldValue) {
            if (!nestedConstruct) {
                visitorReportDiagnostic(visitor, definition, "E2015", "Failed to allocate a default field value.");
            }

            valueFree(instance);
            return (void*)0;
        }

        // Store every initialized field, regardless of its type.
        valueConstructSetField(instance, fieldName, fieldValue);
    }

    return instance;
}

/*
 * Creates a construct instance, initializes its default fields, and
 * applies the explicitly provided field initializers.
 */

value_T* visitorVisitConstructInstantiation(visitor_T* visitor, ast_T* node) {
    ast_T* definition = scopeGetQualifiedConstructDefinition(
        node->scope,
        node->constructInstantiationName
    );

    if (!definition) {
        visitorReportDiagnostic(visitor, node, "E2016", "Undefined construct.");
        return (void*)0;
    }

	const char* typeStack[64] = {0};

	value_T* instance = visitorCreateDefaultConstruct(visitor, node->scope, definition, typeStack, 0);
	
	if (!instance) { return (void*)0; }
	
	free(instance->constructValue.name);

	instance->constructValue.name = calloc(strlen(node->constructInstantiationName) + 1, sizeof(char));

	if (!instance->constructValue.name) {
		visitorReportDiagnostic(visitor, node, "E2019", "Failed to allocate instance name.");
		valueFree(instance);
		return (void*)0;
	}

	strcpy(instance->constructValue.name, node->constructInstantiationName);

    // Apply the explicitly provided initializers.
    for (size_t i = 0; i < node->constructInstantiationFieldsSize; ++i) {
        char* fieldName = node->constructInstantiationFieldNames[i];
        ast_T* fieldExpression = node->constructInstantiationFieldValues[i];
        
		// Reject duplicate initializers.
		for (size_t j = 0; j < i; ++j) {
			if (strcmp(node->constructInstantiationFieldNames[j], fieldName) == 0) {
				visitorReportDiagnostic(visitor, node, "E2017", "Duplicate field initializer in construct instantiation.");
				valueFree(instance);
				return (void*)0;
			}
		}

        // Make sure the field exists and find its declared type.
        size_t fieldIndex = 0;

        while (fieldIndex < definition->constructDefinitionFieldsSize && strcmp(definition->constructDefinitionFields[fieldIndex], fieldName) != 0) {
            ++fieldIndex;
        }

        if (fieldIndex == definition->constructDefinitionFieldsSize) {
            visitorReportDiagnostic(visitor, node, "E2010", "Construct does not contain the requested field.");
            valueFree(instance);
            return (void*)0;
        }

        char* declaredType = definition->constructDefinitionFieldTypes[fieldIndex];

        // Evaluate the initializer expression.
        value_T* fieldValue = visitorVisit(visitor, fieldExpression);

        if (!fieldValue) {
            valueFree(instance);
            return (void*)0;
        }

        // Enforce the declared field type.
        bool typeMatches = true;
        
        if (strcmp(declaredType, "auto") == 0) {
			typeMatches = true;
		}
		else if (strcmp(declaredType, "int") == 0) {
			typeMatches = fieldValue->type == VALUE_INT;
		}
		else if (strcmp(declaredType, "float") == 0) {
			typeMatches = fieldValue->type == VALUE_FLOAT;
		}
		else if (strcmp(declaredType, "string") == 0) {
			typeMatches = fieldValue->type == VALUE_STRING;
		}
		else if (strcmp(declaredType, "array") == 0) {
			typeMatches = fieldValue->type == VALUE_ARRAY;
		}
		else {
			typeMatches = fieldValue->type == VALUE_CONSTRUCT && 
						  fieldValue->constructValue.name && 
						  strcmp(fieldValue->constructValue.name, declaredType) == 0;
		}

		if (!typeMatches) {
			visitorReportDiagnostic(visitor, node, "E2018", "Field initializer has a value incompatible with its declared type.");
			valueFree(fieldValue);
			valueFree(instance);
			return (void*)0;
		}

        // valueConstructSetField takes ownership of fieldValue.
        valueConstructSetField(instance, fieldName, fieldValue);
    }

    return instance;
}
