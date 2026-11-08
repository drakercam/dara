#include "../include/builtin_registry.h"
#include "../include/builtin_functions.h"

#include <stdio.h>

static value_T* builtinFunctionTypeofAdapter(visitor_T* visitor, ast_T** args, int argsSize) {
	if (argsSize != 1) {
		visitorReportDiagnostic(
			visitor,
			argsSize > 0 ? args[0] : NULL,
			"E2033",
			"typeof expects exactly one argument."
		);

		return NULL;
	}

	return builtinFunctionTypeof(visitor, args[0]);
}

/*
 * Starts or restarts the visitor's performance timer.
 */
static value_T* builtinFunctionTimerBegin(visitor_T* visitor, ast_T** args, int argsSize) {
    if (argsSize != 0) {
        visitorReportDiagnostic(
            visitor,
            argsSize > 0 ? args[0] : NULL,
            "E2034",
            "timer_begin expects no arguments."
        );

        return NULL;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &visitor->timerStart) != 0) {
        visitorReportDiagnostic(
            visitor,
            NULL,
            "E2036",
            "Failed to start the performance timer."
        );

        return NULL;
    }

    visitor->timerStarted = true;

    return valueInit(VALUE_NULL);
}

/*
 * Ends the visitor's performance timer and prints elapsed time.
 */
static value_T* builtinFunctionTimerEnd(visitor_T* visitor, ast_T** args, int argsSize) {
    if (argsSize != 0) {
        visitorReportDiagnostic(
            visitor,
            argsSize > 0 ? args[0] : NULL,
            "E2034",
            "timer_end expects no arguments."
        );

        return NULL;
    }

    if (!visitor->timerStarted) {
        visitorReportDiagnostic(
            visitor,
            NULL,
            "E2035",
            "timer_end called before timer_begin."
        );

        return NULL;
    }

    struct timespec end;

    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        visitorReportDiagnostic(
            visitor,
            NULL,
            "E2036",
            "Failed to read the performance timer."
        );

        return NULL;
    }

    double elapsed =
        (double)(end.tv_sec - visitor->timerStart.tv_sec) +
        (double)(end.tv_nsec - visitor->timerStart.tv_nsec) / 1000000000.0;

    printf(
        "Elapsed time: %.6f seconds (%.3f ms)\n",
        elapsed,
        elapsed * 1000.0
    );

    visitor->timerStarted = false;

    return valueInit(VALUE_NULL);
}

void builtinRegisterUtils(scope_T* scope) {
	scopeRegisterNativeFunction(scope, "std:util.typeof", builtinFunctionTypeofAdapter);
	scopeRegisterNativeFunction(scope, "std:util.timer_begin", builtinFunctionTimerBegin);
	scopeRegisterNativeFunction(scope, "std:util.timer_end", builtinFunctionTimerEnd);
}
