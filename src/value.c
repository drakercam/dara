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
			
        case VALUE_NUMBER:
            break;

        case VALUE_STRING:
            free(value->stringValue);
            break;
            
        case VALUE_BOOLEAN:
			break;

        case VALUE_STRUCT:
            for (size_t i = 0;
                 i < value->structValue.fieldsSize;
                 ++i) {

                valueField_T* field =
                    &value->structValue.fields[i];

                free(field->name);
                valueFree(field->value);
            }

            free(value->structValue.fields);
            break;
    }

    free(value);
}

value_T* valueInitNumber(double number) {
	value_T* value = valueInit(VALUE_NUMBER);
	
	value->numberValue = number;
	
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

value_T* valueInitStruct(void) {
	value_T* value = valueInit(VALUE_STRUCT);
	
	value->structValue.fields = (void*)0;
	value->structValue.fieldsSize = 0;
	
	return value;
}

value_T* valueStructSetField(value_T* value, const char* name, value_T* fieldValue) {
	if (value->type != VALUE_STRUCT) {
		return (void*)0;
	}
	
	for (size_t i = 0; i < value->structValue.fieldsSize; ++i) {
		valueField_T* field =
			&value->structValue.fields[i];

		if (strcmp(field->name, name) == 0) {
			valueFree(field->value);
			field->value = fieldValue;

			return fieldValue;
		}
	}
	
	size_t fieldIndex = value->structValue.fieldsSize;
	
	value->structValue.fields = realloc(
					value->structValue.fields, 
					(fieldIndex + 1) * sizeof(valueField_T)
	);
	
	valueField_T* field = &value->structValue.fields[fieldIndex];
	
	field->name = calloc(strlen(name) + 1, sizeof(char));
	strcpy(field->name, name);
	
	field->value = fieldValue;
	
	value->structValue.fieldsSize += 1;
	
	return fieldValue;
}

value_T* valueStructGetField(value_T* value, const char* name) {
	if (value->type != VALUE_STRUCT) {
		return (void*)0;
	}
	
	for (size_t i = 0; i < value->structValue.fieldsSize; ++i) {
		valueField_T* field = &value->structValue.fields[i];
		
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

		case VALUE_NUMBER:
			return valueInitNumber(value->numberValue);
			
		case VALUE_BOOLEAN:
			return valueInitBoolean(value->booleanValue);

		case VALUE_STRING:
			return valueInitString(value->stringValue);

		case VALUE_STRUCT: {
			value_T* copy = valueInitStruct();

			for (size_t i = 0; i < value->structValue.fieldsSize; ++i) {
				valueField_T* field = &value->structValue.fields[i];

				valueStructSetField(
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
