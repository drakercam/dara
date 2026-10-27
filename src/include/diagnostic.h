#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "source.h"
#include <stdbool.h>

/*
 * Categories of diagnostics that can occur whilst interpreting dara code
 */
 
 typedef enum DIAGNOSTIC_CATEGORY {
	 DIAGNOSTIC_LEXER,
	 DIAGNOSTIC_PARSER,
	 DIAGNOSTIC_RUNTIME
	 
 } diagnosticCategory_T;
 
/*
 * Represents an error or warning associated with a location in the dara source code 
*/

typedef struct DIAGNOSTIC_STRUCT {
	
	diagnosticCategory_T category;
	char* code;
	char* message;
	sourceLocation_T location;
	
} diagnostic_T;

/*
 *  creates a diagnostic error or warning
 */ 

diagnostic_T* diagnosticInit(diagnosticCategory_T category, const char* code, const char* message, sourceLocation_T location);
void diagnosticPrint(const diagnostic_T* diagnostic, const char* filename);

void diagnosticFree(diagnostic_T* diagnostic);

#endif
