#ifndef SOURCE_H
#define SOURCE_H

#include <stddef.h>

typedef struct SOURCE_LOCATION_STRUCT {
	
	size_t line;
	size_t column;
	size_t length;
	
} sourceLocation_T;

#endif
