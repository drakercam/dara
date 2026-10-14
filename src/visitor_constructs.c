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

value_T* visitorCreateDefaultConstruct(scope_T* scope, ast_T* definition, const char** typeStack, size_t depth) {
    // Prevent infinite recursion from cyclic construct fields.
    if (depth >= 64) {
        printf("Construct nesting exceeds maximum depth\n");
        exit(1);
    }

    const char* name = definition->constructDefinitionName;

    for (size_t i = 0; i < depth; ++i) {
        if (strcmp(typeStack[i], name) == 0) {
            printf("Recursive construct field type '%s'\n", name);
            exit(1);
        }
    }

    typeStack[depth] = name;

    value_T* instance = valueInitConstruct(name);

    for (size_t i = 0;
         i < definition->constructDefinitionFieldsSize;
         ++i) {

        char* fieldName = definition->constructDefinitionFields[i];
        char* fieldType = definition->constructDefinitionFieldTypes[i];

        value_T* fieldValue = (void*)0;

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
        else {
            ast_T* nestedDefinition =
                scopeGetConstructDefinition(scope, fieldType);

            if (nestedDefinition == (void*)0) {
                printf("Unknown field type '%s'\n", fieldType);
                valueFree(instance);
                exit(1);
            }

            fieldValue = visitorCreateDefaultConstruct(
                scope,
                nestedDefinition,
                typeStack,
                depth + 1
            );
        }

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

    if (definition == (void*)0) {
        printf("Undefined construct %s\n", node->constructInstantiationName);
        exit(1);
    }

	const char* typeStack[64] = {0};

	value_T* instance = visitorCreateDefaultConstruct(
		node->scope,
		definition,
		typeStack,
		0
	);
	
	free(instance->constructValue.name);

	instance->constructValue.name = calloc(
		strlen(node->constructInstantiationName) + 1,
		sizeof(char)
	);

	if (instance->constructValue.name == (void*)0) {
		printf("Failed to allocate construct name\n");
		valueFree(instance);
		exit(EXIT_FAILURE);
	}

	strcpy(
		instance->constructValue.name,
		node->constructInstantiationName
	);

    // Apply the explicitly provided initializers.
    for (size_t i = 0; i < node->constructInstantiationFieldsSize; ++i) {
        char* fieldName =
            node->constructInstantiationFieldNames[i];

        ast_T* fieldExpression =
            node->constructInstantiationFieldValues[i];
            
            // Reject duplicate initializers.
			for (size_t j = 0; j < i; ++j) {
				if (strcmp(
						node->constructInstantiationFieldNames[j],
						fieldName
					) == 0) {

					printf(
						"Duplicate initializer for field '%s' in construct '%s'\n",
						fieldName,
						node->constructInstantiationName
					);

					valueFree(instance);
					exit(1);
				}
			}

        // Make sure the field exists and find its declared type.
        size_t fieldIndex = 0;

        while (fieldIndex < definition->constructDefinitionFieldsSize &&
               strcmp(
                   definition->constructDefinitionFields[fieldIndex],
                   fieldName
               ) != 0) {
            ++fieldIndex;
        }

        if (fieldIndex == definition->constructDefinitionFieldsSize) {
            printf(
                "Construct '%s' has no field named '%s'\n",
                node->constructInstantiationName,
                fieldName
            );
            valueFree(instance);
            exit(1);
        }

        char* declaredType =
            definition->constructDefinitionFieldTypes[fieldIndex];

        // Evaluate the initializer expression.
        value_T* fieldValue = visitorVisit(visitor, fieldExpression);

        if (fieldValue == (void*)0) {
            printf("Failed to evaluate initializer for field '%s'\n", fieldName);
            valueFree(instance);
            exit(1);
        }

        // Enforce the declared field type.
        if ((strcmp(declaredType, "int") == 0 &&
             fieldValue->type != VALUE_INT) ||
            (strcmp(declaredType, "float") == 0 &&
             fieldValue->type != VALUE_FLOAT) ||
            (strcmp(declaredType, "string") == 0 &&
             fieldValue->type != VALUE_STRING)) {

            printf(
                "Type mismatch for field '%s' in construct '%s'\n",
                fieldName,
                node->constructInstantiationName
            );

            valueFree(fieldValue);
            valueFree(instance);
            exit(1);
        }

        // valueConstructSetField takes ownership of fieldValue.
        valueConstructSetField(instance, fieldName, fieldValue);
    }

    return instance;
}
