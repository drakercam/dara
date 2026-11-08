#ifndef PROFILER_H
#define PROFILER_H

#include <stddef.h>
#include <stdio.h>

#ifdef PROFILE

/* AST visitation */
void profilerRecordASTVisit(int nodeType);

/* Function dispatch */
void profilerRecordNativeCall(void);
void profilerRecordDaraCall(void);
void profilerRecordArrayCall(void);

/* Value profiling */
void profilerRecordValueAllocation(size_t size);
void profilerRecordValueFree(void);

/* Scope profiling */
void profilerRecordVariableLookup(void);
void profilerRecordVariableScope(size_t depth);
void profilerRecordVariableCompare(void);

void profilerRecordCallableLookup(void);
void profilerRecordCallableScope(size_t depth);
void profilerRecordCallableCompare(void);

/* Reporting */
void profilerReport(FILE* output);
void profilerReset(void);

/* Value macros */
#define PROFILE_VALUE_ALLOC(size) \
    profilerRecordValueAllocation(size)

#define PROFILE_VALUE_FREE() \
    profilerRecordValueFree()

/* Variable macros */
#define PROFILE_VARIABLE_LOOKUP() \
    profilerRecordVariableLookup()

#define PROFILE_VARIABLE_SCOPE(depth) \
    profilerRecordVariableScope(depth)

#define PROFILE_VARIABLE_COMPARE() \
    profilerRecordVariableCompare()

/* Callable macros */
#define PROFILE_CALLABLE_LOOKUP() \
    profilerRecordCallableLookup()

#define PROFILE_CALLABLE_SCOPE(depth) \
    profilerRecordCallableScope(depth)

#define PROFILE_CALLABLE_COMPARE() \
    profilerRecordCallableCompare()
    
#define PROFILE_AST_VISIT(type) \
    profilerRecordASTVisit(type)

#define PROFILE_NATIVE_CALL() \
    profilerRecordNativeCall()

#define PROFILE_DARA_CALL() \
    profilerRecordDaraCall()

#define PROFILE_ARRAY_CALL() \
    profilerRecordArrayCall()

#else

/* No-op macros when profiling is disabled */
#define PROFILE_VALUE_ALLOC(size) ((void)0)
#define PROFILE_VALUE_FREE() ((void)0)

#define PROFILE_VARIABLE_LOOKUP() ((void)0)
#define PROFILE_VARIABLE_SCOPE(depth) ((void)0)
#define PROFILE_VARIABLE_COMPARE() ((void)0)

#define PROFILE_CALLABLE_LOOKUP() ((void)0)
#define PROFILE_CALLABLE_SCOPE(depth) ((void)0)
#define PROFILE_CALLABLE_COMPARE() ((void)0)

#define PROFILE_AST_VISIT(type) ((void)0)

#define PROFILE_NATIVE_CALL() ((void)0)
#define PROFILE_DARA_CALL() ((void)0)
#define PROFILE_ARRAY_CALL() ((void)0)

#endif

#endif
