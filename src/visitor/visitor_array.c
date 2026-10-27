#include "../include/visitor.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

/*
 * Creates an array with a specified capacity and element type.
 */
value_T* visitorVisitArrayConstructor(visitor_T* visitor, ast_T* node) {
    size_t capacity = 1;

    // Evaluate the optional capacity expression.
    if (node->arrayConstructorCapacity != (void*)0) {
        value_T* capacityValue = visitorVisit(visitor, node->arrayConstructorCapacity);

        if (capacityValue == (void*)0 || capacityValue->type != VALUE_INT) {
            visitorReportDiagnostic(visitor, node, "E2049", "Array capacity must be a positive integer.");

            valueFree(capacityValue);
            return (void*)0;
        }

        if (capacityValue->intValue <= 0) {
            visitorReportDiagnostic(visitor, node, "E2049", "Array capacity must be a positive integer.");

            valueFree(capacityValue);
            return (void*)0;
        }

        capacity = (size_t)capacityValue->intValue;
        valueFree(capacityValue);
    }

    const char* elementTypeName = node->arrayConstructorElementType;
    int elementType;

    if (strcmp(elementTypeName, "auto") == 0) {
        // VALUE_NULL is the placeholder for an unconstrained element type.
        elementType = VALUE_NULL;
    } else if (strcmp(elementTypeName, "int") == 0) {
        elementType = VALUE_INT;
    } else if (strcmp(elementTypeName, "float") == 0) {
        elementType = VALUE_FLOAT;
    } else if (strcmp(elementTypeName, "string") == 0) {
        elementType = VALUE_STRING;
    } else if (strcmp(elementTypeName, "boolean") == 0) {
        elementType = VALUE_BOOLEAN;
    } else {
        visitorReportDiagnostic(visitor, node, "E2003", "Unsupported array element type.");

        return (void*)0;
    }

    array_T* array = calloc(1, sizeof(array_T));

    if (array == (void*)0) {
        visitorReportDiagnostic(visitor, node, "E2004","Failed to allocate array.");

        return (void*)0;
    }

    array->elements = calloc(capacity, sizeof(value_T*));

    if (array->elements == (void*)0) {
        free(array);

        visitorReportDiagnostic(visitor, node, "E2004", "Failed to allocate array elements.");

        return (void*)0;
    }

    array->elementTypeName = calloc(strlen(elementTypeName) + 1, sizeof(char));

    if (array->elementTypeName == (void*)0) {
        free(array->elements);
        free(array);

        visitorReportDiagnostic(visitor, node, "E2004", "Failed to allocate array element type.");

        return (void*)0;
    }

    strcpy(array->elementTypeName, elementTypeName);

    array->size = 0;
    array->capacity = capacity;
    array->elementType = elementType;

    value_T* result = valueInit(VALUE_ARRAY);

    if (result == (void*)0) {
        arrayFree(array);
        return (void*)0;
    }

    result->arrayValue = array;

    return result;
}

/*
 * Checks whether a value is compatible with an array's element type.
 */
static bool arrayElementMatchesType(
    const array_T* array,
    const value_T* value
) {
    if (!array || !value) {
        return false;
    }

    // "auto" arrays accept any value type.
    if (array->elementType == VALUE_NULL) {
        return true;
    }

    return array->elementType == value->type;
}

/*
 * Creates a new array with the same element type as its source.
 * Capacity is at least one, even for an empty carved range.
 */
static array_T* arrayCreateLike(
    const array_T* source,
    size_t capacity
) {
    if (!source) {
        return NULL;
    }

    if (capacity == 0) {
        capacity = 1;
    }

    array_T* result = calloc(1, sizeof(array_T));

    if (!result) {
        return NULL;
    }

    result->elements = calloc(capacity, sizeof(value_T*));

    if (!result->elements) {
        free(result);
        return NULL;
    }

    size_t typeNameLength = strlen(source->elementTypeName);

    result->elementTypeName = calloc(
        typeNameLength + 1,
        sizeof(char)
    );

    if (!result->elementTypeName) {
        free(result->elements);
        free(result);
        return NULL;
    }

    memcpy(
        result->elementTypeName,
        source->elementTypeName,
        typeNameLength + 1
    );

    result->elementType = source->elementType;
    result->capacity = capacity;
    result->size = 0;

    return result;
}

/*
 * Evaluates an expression that must produce an array.
 * The returned value is an owned temporary and must be freed.
 */
static value_T* arrayEvaluateArgument(
    visitor_T* visitor,
    ast_T* node
) {
    value_T* value = visitorVisit(visitor, node);

    if (!value) {
        return (void*)0;
    }

    if (value->type != VALUE_ARRAY || !value->arrayValue) {
        visitorReportDiagnostic(
            visitor,
            node,
            "E2006",
            "Array operation requires a valid array argument."
        );

        valueFree(value);
        return (void*)0;
    }

    return value;
}

/*
 * Evaluates an index expression and rejects non-integer or negative
 * indices. Bounds against a particular array are checked separately.
 */
static bool arrayEvaluateIndex(visitor_T* visitor, ast_T* node, size_t* index) {
    value_T* value = visitorVisit(visitor, node);

    if (!value) {
        return false;
    }

    if (value->type != VALUE_INT) {
        visitorReportDiagnostic(
            visitor,
            node,
            "E2041",
            "Array index must be an integer."
        );

        valueFree(value);
        return false;
    }

    if (value->intValue < 0) {
        visitorReportDiagnostic(
            visitor,
            node,
            "E2042",
            "Array index cannot be negative."
        );

        valueFree(value);
        return false;
    }

    *index = (size_t)value->intValue;

    valueFree(value);
    return true;
}

/*
 * Resolves a mutable array argument to its actual stored value.
 * This avoids modifying the temporary copy returned by variable lookup.
 */
static value_T* arrayGetMutableArgument(visitor_T* visitor, ast_T* node) {
    if (!node || (node->type != AST_VARIABLE && node->type != AST_MEMBER_ACCESS)) {
        visitorReportDiagnostic(
            visitor,
            node,
            "E2043",
            "Array mutation requires an assignable array variable or field."
        );

        return (void*)0;
    }

    value_T* value = visitorGetAssignmentTargetValue(visitor, node);

    if (!value) {
        return (void*)0;
    }

    if (value->type != VALUE_ARRAY || !value->arrayValue) {
        visitorReportDiagnostic(
            visitor,
            node,
            "E2006",
            "Array operation requires a valid array argument."
        );

        return (void*)0;
    }

    return value;
}

/*
 * Multiplies an array's capacity by the specified growth factor.
 * Existing elements are preserved, and new slots are initialized to NULL.
 */
static bool arrayGrowCapacity(array_T* array, size_t factor) {
	if (!array || factor < 2) {
		return false;
	}

	if (array->capacity > SIZE_MAX / factor) {
		return false;
	}

	size_t newCapacity = array->capacity * factor;

	if (newCapacity > SIZE_MAX / sizeof(value_T*)) {
		return false;
	}

	value_T** elements = realloc(
		array->elements,
		newCapacity * sizeof(value_T*)
	);

	if (!elements) {
		return false;
	}

	memset(
		elements + array->capacity,
		0,
		(newCapacity - array->capacity) * sizeof(value_T*)
	);

	array->elements = elements;
	array->capacity = newCapacity;

	return true;
}

/*
 * Evaluates a method call belonging to the std:array namespace.
 */
value_T* visitorVisitArrayMethodCall(visitor_T* visitor, ast_T* node) {
	if (strcmp(node->methodCallName, "size") == 0 ||
		strcmp(node->methodCallName, "capacity") == 0 ||
		strcmp(node->methodCallName, "add") == 0 ||
		strcmp(node->methodCallName, "at") == 0 ||
		strcmp(node->methodCallName, "carve") == 0 ||
		strcmp(node->methodCallName, "mend") == 0 ||
		strcmp(node->methodCallName, "erase") == 0 ||
		strcmp(node->methodCallName, "grow") == 0) {

		int argCount = node->methodCallArgumentsSize;
		const char* methodName = node->methodCallName;

		if (argCount != 1 &&
			(strcmp(methodName, "size") == 0 ||
			 strcmp(methodName, "capacity") == 0)) {
			visitorReportDiagnostic(
				visitor, node, "E2005",
				"Array size and capacity methods expect exactly one argument."
			);
			return (void*)0;
		}

		if ((strcmp(methodName, "add") == 0 ||
			 strcmp(methodName, "at") == 0 ||
			 strcmp(methodName, "erase") == 0 ||
			 strcmp(methodName, "grow") == 0) &&
			argCount != 2) {
			visitorReportDiagnostic(
				visitor, node, "E2005",
				"Array add, at, erase, and grow methods expect exactly two arguments."
			);
			return (void*)0;
		}

		if (strcmp(methodName, "carve") == 0 && argCount != 3) {
			visitorReportDiagnostic(
				visitor, node, "E2005",
				"Array carve method expects exactly three arguments."
			);
			return (void*)0;
		}

		if (strcmp(methodName, "mend") == 0 && argCount != 4) {
			visitorReportDiagnostic(
				visitor, node, "E2005",
				"Array mend method expects exactly four arguments."
			);
			return (void*)0;
		}

		/*
		 * Mutating methods must access the actual stored array,
		 * rather than the temporary copy returned by visitorVisit().
		 */
		if (strcmp(methodName, "add") == 0 ||
			strcmp(methodName, "erase") == 0 ||
			strcmp(methodName, "mend") == 0 ||
			strcmp(methodName, "grow") == 0) {

			value_T* argument = arrayGetMutableArgument(
				visitor, node->methodCallArguments[0]
			);

			if (!argument) {
				return (void*)0;
			}

			array_T* array = argument->arrayValue;
			
			if (strcmp(methodName, "grow") == 0) {
				value_T* factorValue = visitorVisit(
					visitor, node->methodCallArguments[1]
				);

				if (!factorValue) {
					return (void*)0;
				}

				if (factorValue->type != VALUE_INT || factorValue->intValue < 2) {
					visitorReportDiagnostic(
						visitor,
						node,
						"E2048",
						"Array growth factor must be an integer greater than or equal to two."
					);

					valueFree(factorValue);
					return (void*)0;
				}

				size_t factor = (size_t)factorValue->intValue;
				valueFree(factorValue);

				if (!arrayGrowCapacity(array, factor)) {
					visitorReportDiagnostic(
						visitor,
						node,
						"E2004",
						"Failed to grow array or requested capacity is too large."
					);
					return (void*)0;
				}

				return valueInit(VALUE_NULL);
			}

			if (strcmp(methodName, "add") == 0) {
				value_T* value = visitorVisit(
					visitor, node->methodCallArguments[1]
				);

				if (!value) {
					return (void*)0;
				}

				if (!arrayElementMatchesType(array, value)) {
					visitorReportDiagnostic(
						visitor, node, "E2044",
						"Value type does not match the array element type."
					);
					valueFree(value);
					return (void*)0;
				}

				if (array->size >= array->capacity) {
					visitorReportDiagnostic(
						visitor, node, "E2045",
						"Array capacity exceeded."
					);
					valueFree(value);
					return (void*)0;
				}

				array->elements[array->size] = valueCopy(value);
				valueFree(value);

				if (!array->elements[array->size]) {
					visitorReportDiagnostic(
						visitor,
						node,
						"E2004",
						"Failed to copy array element."
					);
					return (void*)0;
				}

				array->size++;

				return valueInit(VALUE_NULL);
			}

			if (strcmp(methodName, "erase") == 0) {
				size_t index;

				if (!arrayEvaluateIndex(
					visitor, node->methodCallArguments[1], &index
				)) {
					return (void*)0;
				}

				if (index >= array->size) {
					visitorReportDiagnostic(
						visitor, node, "E2041",
						"Array index is out of bounds."
					);
					return (void*)0;
				}

				valueFree(array->elements[index]);

				for (size_t i = index; i + 1 < array->size; i++) {
					array->elements[i] = array->elements[i + 1];
				}

				array->size--;
				array->elements[array->size] = NULL;

				return valueInit(VALUE_NULL);
			}

			/* mend(destination, source, start, end) */
			value_T* sourceValue = arrayEvaluateArgument(
				visitor, node->methodCallArguments[1]
			);

			if (!sourceValue) {
				return (void*)0;
			}

			array_T* source = sourceValue->arrayValue;
			size_t start;
			size_t end;

			if (!arrayEvaluateIndex(
					visitor, node->methodCallArguments[2], &start
				) ||
				!arrayEvaluateIndex(
					visitor, node->methodCallArguments[3], &end
				)) {
				valueFree(sourceValue);
				return (void*)0;
			}

			if (start > end || end > source->size) {
				visitorReportDiagnostic(
					visitor, node, "E2047",
					"Invalid array range for mend."
				);
				valueFree(sourceValue);
				return (void*)0;
			}

			size_t count = end - start;

			if (count > array->capacity - array->size) {
				visitorReportDiagnostic(
					visitor, node, "E2045",
					"Array capacity exceeded."
				);
				valueFree(sourceValue);
				return (void*)0;
			}

			for (size_t i = start; i < end; i++) {
				if (!arrayElementMatchesType(array, source->elements[i])) {
					visitorReportDiagnostic(
						visitor, node, "E2044",
						"Source element type does not match the destination array type."
					);
					valueFree(sourceValue);
					return (void*)0;
				}
			}

			/*
			 * Copy into temporary storage first. This also makes
			 * mend(array, array, start, end) safe.
			 */
			value_T** copies = calloc(count ? count : 1, sizeof(value_T*));

			if (!copies) {
				valueFree(sourceValue);
				visitorReportDiagnostic(
					visitor, node, "E2004",
					"Failed to allocate array elements."
				);
				return (void*)0;
			}

			for (size_t i = 0; i < count; i++) {
				copies[i] = valueCopy(source->elements[start + i]);

				if (!copies[i]) {
					for (size_t j = 0; j < i; j++) {
						valueFree(copies[j]);
					}

					free(copies);
					valueFree(sourceValue);

					visitorReportDiagnostic(
						visitor,
						node,
						"E2004",
						"Failed to copy elements while appending an array range."
					);

					return (void*)0;
				}
			}

			for (size_t i = 0; i < count; i++) {
				array->elements[array->size + i] = copies[i];
			}

			array->size += count;

			free(copies);
			valueFree(sourceValue);

			return valueInit(VALUE_NULL);
		}

		/*
		 * Read-only methods evaluate their array argument normally.
		 * Variable evaluation returns a deep copy, so these operations
		 * cannot accidentally modify the stored array.
		 */
		value_T* argument = arrayEvaluateArgument(
			visitor, node->methodCallArguments[0]
		);

		if (!argument) {
			return (void*)0;
		}

		array_T* array = argument->arrayValue;

		if (strcmp(methodName, "size") == 0 ||
			strcmp(methodName, "capacity") == 0) {

			size_t resultValue = strcmp(methodName, "size") == 0
				? array->size
				: array->capacity;

			valueFree(argument);

			if (resultValue > (size_t)INT_MAX) {
				visitorReportDiagnostic(
					visitor, node, "E2007",
					"Array result exceeds supported integer range."
				);
				return (void*)0;
			}

			value_T* result = valueInit(VALUE_INT);
			result->intValue = (int)resultValue;

			return result;
		}

		if (strcmp(methodName, "at") == 0) {
			size_t index;

			if (!arrayEvaluateIndex(
				visitor, node->methodCallArguments[1], &index
			)) {
				valueFree(argument);
				return (void*)0;
			}

			if (index >= array->size) {
				visitorReportDiagnostic(
					visitor, node, "E2046",
					"Array index is out of bounds."
				);
				valueFree(argument);
				return (void*)0;
			}

			value_T* result = valueCopy(array->elements[index]);

			valueFree(argument);
			return result;
		}

		/* carve(source, start, end) returns a new array. */
		if (strcmp(methodName, "carve") == 0) {
			size_t start;
			size_t end;

			if (!arrayEvaluateIndex(
					visitor, node->methodCallArguments[1], &start
				) ||
				!arrayEvaluateIndex(
					visitor, node->methodCallArguments[2], &end
				)) {
				valueFree(argument);
				return (void*)0;
			}

			if (start > end || end > array->size) {
				visitorReportDiagnostic(
					visitor, node, "E2047",
					"Invalid array range for carve."
				);
				valueFree(argument);
				return (void*)0;
			}

			size_t count = end - start;
			array_T* carved = arrayCreateLike(array, count);

			if (!carved) {
				valueFree(argument);
				visitorReportDiagnostic(
					visitor, node, "E2004",
					"Failed to allocate carved array."
				);
				return (void*)0;
			}

			for (size_t i = 0; i < count; i++) {
				carved->elements[i] = valueCopy(array->elements[start + i]);

				if (!carved->elements[i]) {
					arrayFree(carved);
					valueFree(argument);

					visitorReportDiagnostic(
						visitor,
						node,
						"E2004",
						"Failed to copy an element while carving an array."
					);

					return (void*)0;
				}

				carved->size++;
			}

			value_T* result = valueInit(VALUE_ARRAY);
			result->arrayValue = carved;

			valueFree(argument);
			return result;
		}

		valueFree(argument);
	}

	visitorReportDiagnostic(
		visitor, node, "E2008", "Unknown std:array method."
	);

	return (void*)0;
}

