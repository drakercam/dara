#ifndef VALUE_H
#define VALUE_H

#include <stddef.h>

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
		VALUE_STRUCT
		
	} type;
	
	union {
		double numberValue;
		char* stringValue;
		
		struct {
			valueField_T* fields;
			size_t fieldsSize;
			
		} structValue;
	};
};

value_T* valueInit(int type);
void valueFree(value_T* value);

value_T* valueInitNumber(double number);
value_T* valueInitString(const char* string);

value_T* valueInitStruct(void);

value_T* valueStructSetField(value_T* value, const char* name, value_T* fieldValue);
value_T* valueStructGetField(value_T* value, const char* name);

#endif
