#ifndef VALUE_H
#define VALUE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct VALUE_STRUCT value_T;

typedef struct VALUE_FIELD_STRUCT {
	char* name;
	value_T* value;
	
} valueField_T;

struct VALUE_STRUCT {
	enum {
		VALUE_NULL,
		VALUE_INT,
		VALUE_FLOAT,
		VALUE_STRING,
		VALUE_BOOLEAN,
		VALUE_CONSTRUCT
		
	} type;
	
	union {
		int intValue;
		double floatValue;
		char* stringValue;
		bool booleanValue;
		
		struct {
			char* name;
			valueField_T* fields;
			size_t fieldsSize;
			
		} constructValue;
	};
};

value_T* valueInit(int type);
void valueFree(value_T* value);

value_T* valueInitInt(int number);
value_T* valueInitFloat(double number);
value_T* valueInitString(const char* string);
value_T* valueInitBoolean(bool boolean);

value_T* valueInitConstruct(const char* name);

value_T* valueConstructSetField(value_T* value, const char* name, value_T* fieldValue);
value_T* valueConstructGetField(value_T* value, const char* name);

value_T* valueCopy(const value_T* value);
void valueReplace(value_T* destination, value_T* source);

#endif
