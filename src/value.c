#include "include/value.h"
#include "include/builtin_functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

value_T* valueInit(int type) {
	value_T* value = calloc(1, sizeof(value_T));
	
	if (value == (void*)0) {
		printf("Value is null");
		exit(1);
	}
	
	value->type = type;
	
	return value;
}

void arrayFree(array_T* array) {
    if (array == (void*)0) {
        return;
    }

    for (size_t i = 0; i < array->size; ++i) {
        valueFree(array->elements[i]);
    }

    free(array->elements);
    free(array->elementTypeName);
    free(array);
}

static void valueFreeContents(value_T* value) {
    switch (value->type) {
        case VALUE_NULL:
        case VALUE_INT:
        case VALUE_FLOAT:
        case VALUE_BOOLEAN:
        case VALUE_COLOR:
            break;

        case VALUE_STRING:
            free(value->stringValue);
            break;

        case VALUE_CONSTRUCT:
            free(value->constructValue.name);

            for (size_t i = 0;
                 i < value->constructValue.fieldsSize;
                 ++i) {
                valueField_T* field =
                    &value->constructValue.fields[i];

                free(field->name);
                valueFree(field->value);
            }

            free(value->constructValue.fields);
            break;
            
		case VALUE_ARRAY:
			arrayFree(value->arrayValue);
			break;
    }
}

void valueFree(value_T* value) {
    if (value == (void*)0) {
        return;
    }

    valueFreeContents(value);
    free(value);
}

void valueReplace(value_T* destination, value_T* source) {
    if (destination == (void*)0 || source == (void*)0 ||
        destination == source) {
        return;
    }

    // Release the destination's old contents, not its allocation.
    valueFreeContents(destination);

    // Transfer the source's contents into the existing destination.
    memcpy(destination, source, sizeof(value_T));

    // The destination now owns the transferred contents.
    // Free only the source's outer allocation.
    free(source);
}

value_T* valueInitInt(int number) {
	value_T* value = valueInit(VALUE_INT);
	
	value->intValue = number;
	
	return value;
}

value_T* valueInitFloat(double number) {
	value_T* value = valueInit(VALUE_FLOAT);
	
	value->floatValue = number;
	
	return value;
}

value_T* valueInitString(const char* string) {
	value_T* value = valueInit(VALUE_STRING);
	
	value->stringValue = calloc(strlen(string) + 1, sizeof(char));
	strcpy(value->stringValue, string);
	
	return value;
}

value_T* valueInitBoolean(bool boolean) {
	value_T* value = valueInit(VALUE_BOOLEAN);
	
	value->booleanValue = boolean;
	
	return value;
}

value_T* valueInitColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	value_T* value = valueInit(VALUE_COLOR);

    value->colorValue.r = r;
    value->colorValue.g = g;
    value->colorValue.b = b;
    value->colorValue.a = a;

    return value;
}

value_T* valueInitConstruct(const char* name) {
    value_T* value = valueInit(VALUE_CONSTRUCT);

    value->constructValue.name = calloc(
        strlen(name) + 1,
        sizeof(char)
    );

    strcpy(value->constructValue.name, name);

    value->constructValue.fields = (void*)0;
    value->constructValue.fieldsSize = 0;

    return value;
}

value_T* valueConstructSetField(value_T* value, const char* name, value_T* fieldValue) {
	if (value->type != VALUE_CONSTRUCT) {
		return (void*)0;
	}
	
	for (size_t i = 0; i < value->constructValue.fieldsSize; ++i) {
		valueField_T* field =
			&value->constructValue.fields[i];

		if (strcmp(field->name, name) == 0) {
			valueFree(field->value);
			field->value = fieldValue;

			return fieldValue;
		}
	}
	
	size_t fieldIndex = value->constructValue.fieldsSize;
	
	value->constructValue.fields = realloc(
					value->constructValue.fields, 
					(fieldIndex + 1) * sizeof(valueField_T)
	);
	
	valueField_T* field = &value->constructValue.fields[fieldIndex];
	
	field->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(field->name, name);
	
	field->value = fieldValue;
	
	value->constructValue.fieldsSize += 1;
	
	return fieldValue;
}

value_T* valueConstructGetField(value_T* value, const char* name) {
	if (value->type != VALUE_CONSTRUCT) {
		return (void*)0;
	}
	
	for (size_t i = 0; i < value->constructValue.fieldsSize; ++i) {
		valueField_T* field = &value->constructValue.fields[i];
		
		if (strcmp(field->name, name) == 0) {
			return field->value;
		}
	}
	
	return (void*)0;
}

static array_T* arrayCopy(const array_T* source) {
    if (source == (void*)0) {
        return (void*)0;
    }

    array_T* copy = calloc(1, sizeof(array_T));

    if (copy == (void*)0) {
        return (void*)0;
    }

    copy->size = source->size;
    copy->capacity = source->capacity;
    copy->elementType = source->elementType;

    if (source->elementTypeName != (void*)0) {
        size_t nameLength = strlen(source->elementTypeName);

        copy->elementTypeName = calloc(
            nameLength + 1,
            sizeof(char)
        );

        if (copy->elementTypeName == (void*)0) {
            free(copy);
            return (void*)0;
        }

        memcpy(
            copy->elementTypeName,
            source->elementTypeName,
            nameLength + 1
        );
    }

    if (copy->capacity > 0) {
        copy->elements = calloc(
            copy->capacity,
            sizeof(value_T*)
        );

        if (copy->elements == (void*)0) {
            free(copy->elementTypeName);
            free(copy);
            return (void*)0;
        }
    }

    for (size_t i = 0; i < source->size; ++i) {
        copy->elements[i] = valueCopy(source->elements[i]);

        if (copy->elements[i] == (void*)0) {
            arrayFree(copy);
            return (void*)0;
        }
    }

    return copy;
}

value_T* valueCopy(const value_T* value) {
	if (value == (void*)0) {
		return (void*)0;
	}
	
	switch (value->type) {
		case VALUE_NULL:
			return valueInit(VALUE_NULL);

		case VALUE_INT:
			return valueInitInt(value->intValue);

		case VALUE_FLOAT:
			return valueInitFloat(value->floatValue);
			
		case VALUE_BOOLEAN:
			return valueInitBoolean(value->booleanValue);

		case VALUE_STRING:
			return valueInitString(value->stringValue);

		case VALUE_CONSTRUCT: {
			value_T* copy = valueInitConstruct(
				value->constructValue.name
			);

			for (size_t i = 0; i < value->constructValue.fieldsSize; ++i) {
				valueField_T* field = &value->constructValue.fields[i];

				valueConstructSetField(
					copy,
					field->name,
					valueCopy(field->value)
				);
			}

			return copy;
		}
		
		case VALUE_COLOR:
			return valueInitColor(value->colorValue.r, value->colorValue.g, value->colorValue.b, value->colorValue.a);
			
		case VALUE_ARRAY: {
			array_T* array = arrayCopy(value->arrayValue);
			
			if (array == (void*)0) {
				return (void*)0;
			}
			
			value_T* copy = valueInit(VALUE_ARRAY);
			
			if (copy == (void*)0) {
				arrayFree(array);
				return (void*)0;
			}
			
			copy->arrayValue = array;
			
			return copy;
		}
	}
	
	return (void*)0;
}
