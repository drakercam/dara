#ifndef VALUE_H
#define VALUE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct VALUE_STRUCT value_T;
typedef struct ARRAY_STRUCT array_T;

typedef struct VALUE_FIELD_STRUCT {
	char* name;
	value_T* value;
	
} valueField_T;

struct ARRAY_STRUCT {
	value_T** elements;
	size_t size;
	size_t capacity;
	
	int elementType;
	char* elementTypeName;
};

struct VALUE_STRUCT {
	enum {
		VALUE_NULL,
		VALUE_INT,
		VALUE_FLOAT,
		VALUE_STRING,
		VALUE_BOOLEAN,
		VALUE_CONSTRUCT,
		VALUE_COLOR,
		VALUE_ARRAY
		
	} type;
	
	union {
		int intValue;
		double floatValue;
		char* stringValue;
		bool booleanValue;
		array_T* arrayValue;
		
		struct {
			char* name;
			valueField_T* fields;
			size_t fieldsSize;
			
		} constructValue;
		
		struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} colorValue;
	};
};

value_T* valueInit(int type);
void arrayFree(array_T* array);
void valueFree(value_T* value);

value_T* valueInitInt(int number);
value_T* valueInitFloat(double number);
value_T* valueInitString(const char* string);
value_T* valueInitBoolean(bool boolean);
value_T* valueInitColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

value_T* valueInitConstruct(const char* name);

value_T* valueConstructSetField(value_T* value, const char* name, value_T* fieldValue);
value_T* valueConstructGetField(value_T* value, const char* name);

value_T* valueCopy(const value_T* value);
void valueReplace(value_T* destination, value_T* source);

#endif
