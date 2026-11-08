#include "include/profiler.h"
#include "include/ast.h"

#ifdef PROFILE

typedef struct {
	size_t astVisits[AST_ENUM_DEFINITION + 1];
	
	size_t nativeCalls;
	size_t daraCalls;
	size_t arrayCalls;
	
    /* Value statistics */
    size_t valueAllocations;
    size_t valueFrees;
    size_t valueBytesAllocated;
    size_t liveValues;
    size_t peakLiveValues;

    /* Variable lookup statistics */
    size_t variableLookups;
    size_t variableComparisons;
    size_t variableScopesTraversed;
    size_t maxVariableDepth;

    /* Callable lookup statistics */
    size_t callableLookups;
    size_t callableComparisons;
    size_t callableScopesTraversed;
    size_t maxCallableDepth;

} profiler_T;

static profiler_T profiler;

void profilerRecordASTVisit(int nodeType) {
    if (nodeType < 0 || nodeType > AST_ENUM_DEFINITION) {
        return;
    }

    ++profiler.astVisits[nodeType];
}

void profilerRecordNativeCall(void) {
    ++profiler.nativeCalls;
}

void profilerRecordDaraCall(void) {
    ++profiler.daraCalls;
}

void profilerRecordArrayCall(void) {
    ++profiler.arrayCalls;
}

/* ============================================================
 * Value statistics
 * ============================================================ */

void profilerRecordValueAllocation(size_t size) {
    profiler.valueAllocations++;
    profiler.valueBytesAllocated += size;

    profiler.liveValues++;

    if (profiler.liveValues > profiler.peakLiveValues) {
        profiler.peakLiveValues = profiler.liveValues;
    }
}

void profilerRecordValueFree(void) {
    profiler.valueFrees++;

    if (profiler.liveValues > 0) {
        profiler.liveValues--;
    }
}


/* ============================================================
 * Variable lookup statistics
 * ============================================================ */

void profilerRecordVariableLookup(void) {
    profiler.variableLookups++;
}

void profilerRecordVariableScope(size_t depth) {
    profiler.variableScopesTraversed++;

    if (depth > profiler.maxVariableDepth) {
        profiler.maxVariableDepth = depth;
    }
}

void profilerRecordVariableCompare(void) {
    profiler.variableComparisons++;
}


/* ============================================================
 * Callable lookup statistics
 * ============================================================ */

void profilerRecordCallableLookup(void) {
    profiler.callableLookups++;
}

void profilerRecordCallableScope(size_t depth) {
    profiler.callableScopesTraversed++;

    if (depth > profiler.maxCallableDepth) {
        profiler.maxCallableDepth = depth;
    }
}

void profilerRecordCallableCompare(void) {
    profiler.callableComparisons++;
}


/* ============================================================
 * Reporting
 * ============================================================ */

void profilerReport(FILE* output) {
    if (output == NULL) {
        return;
    }

    fprintf(output, "\n");
    fprintf(output, "========== DARA PROFILER ==========\n");

    fprintf(output, "\n--- Values ---\n");
    fprintf(output, "Allocations:          %zu\n",
        profiler.valueAllocations);
    fprintf(output, "Frees:                %zu\n",
        profiler.valueFrees);
    fprintf(output, "Bytes allocated:      %zu\n",
        profiler.valueBytesAllocated);
    fprintf(output, "Live values:          %zu\n",
        profiler.liveValues);
    fprintf(output, "Peak live values:     %zu\n",
        profiler.peakLiveValues);

    fprintf(output, "\n--- Variable Lookups ---\n");
    fprintf(output, "Lookups:              %zu\n",
        profiler.variableLookups);
    fprintf(output, "Comparisons:          %zu\n",
        profiler.variableComparisons);
    fprintf(output, "Scopes traversed:     %zu\n",
        profiler.variableScopesTraversed);
    fprintf(output, "Maximum depth:        %zu\n",
        profiler.maxVariableDepth);

    fprintf(output, "\n--- Callable Lookups ---\n");
    fprintf(output, "Lookups:              %zu\n",
        profiler.callableLookups);
    fprintf(output, "Comparisons:          %zu\n",
        profiler.callableComparisons);
    fprintf(output, "Scopes traversed:     %zu\n",
        profiler.callableScopesTraversed);
    fprintf(output, "Maximum depth:        %zu\n",
        profiler.maxCallableDepth);
        
    printf("\n--- Function Dispatch ---\n");
	printf("Native calls:          %zu\n", profiler.nativeCalls);
	printf("Dara calls:            %zu\n", profiler.daraCalls);
	printf("Array calls:           %zu\n", profiler.arrayCalls);
        
    static const char* astNodeNames[] = {
		"Variable definition",
		"Function definition",
		"Construct definition",
		"Namespace definition",
		"Variable",
		"Function call",
		"Array constructor",
		"Method call",
		"String",
		"Compound",
		"No-op",
		"Number",
		"Boolean",
		"Unary operation",
		"Binary operation",
		"Return",
		"Construct instantiation",
		"Member access",
		"If statement",
		"While loop",
		"Assignment",
		"For loop",
		"Switch statement",
		"Enum definition"
	};

	printf("\n--- AST Visits ---\n");

	for (size_t i = 0; i < sizeof(astNodeNames) / sizeof(astNodeNames[0]); ++i) {
		printf("%-25s %zu\n", astNodeNames[i], profiler.astVisits[i]);
	}

    fprintf(output, "\n===================================\n");
}

/* ============================================================
 * Reset
 * ============================================================ */

void profilerReset(void) {
    profiler = (profiler_T){0};
}

#endif
