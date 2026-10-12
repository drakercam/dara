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
		VALUE_NUMBER,
		VALUE_STRING,
		VALUE_BOOLEAN,
		VALUE_CONSTRUCT
		
	} type;
	
	union {
		double numberValue;
		char* stringValue;
		bool booleanValue;
		
		struct {
			valueField_T* fields;
			size_t fieldsSize;
			
		} constructValue;
	};
};

value_T* valueInit(int type);
void valueFree(value_T* value);

value_T* valueInitNumber(double number);
value_T* valueInitString(const char* string);
value_T* valueInitBoolean(bool boolean);

value_T* valueInitConstruct(void);

value_T* valueConstructSetField(value_T* value, const char* name, value_T* fieldValue);
value_T* valueConstructGetField(value_T* value, const char* name);

value_T* valueCopy(const value_T* value);

#endif
