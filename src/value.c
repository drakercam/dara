#include "include/value.h"

#include <stdlib.h>
#include <string.h>

value_T* valueInit(int type) {
	value_T* value = calloc(1, sizeof(value_T));
	
	value->type = type;
	
	return value;
}

void valueFree(value_T* value)
{
    if (value == NULL) {
        return;
    }

    switch (value->type) {

        case VALUE_NULL:
			break;
			
        case VALUE_INT:
            break;
            
        case VALUE_FLOAT:
			break;

        case VALUE_STRING:
            free(value->stringValue);
            break;
            
        case VALUE_BOOLEAN:
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
    }

    free(value);
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
	}
	
	return (void*)0;
}
