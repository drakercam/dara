#include "include/visitor.h"
#include "include/scope.h"
#include "include/value.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * Evaluates an if statement, executing its body when the condition is true
 * or its else body when the condition is false.
 */
 
 value_T* visitorVisitIfStatement(visitor_T* visitor, ast_T* node) {
    value_T* condition = visitorVisit(visitor, node->ifCondition);

    if (condition == (void*)0) {
        printf("Failed to evaluate if condition\n");
        exit(1);
    }

    if (condition->type != VALUE_BOOLEAN) {
        printf("If condition must evaluate to a boolean\n");
        valueFree(condition);
        exit(1);
    }

    bool result = condition->booleanValue;
    valueFree(condition);

    if (result) {
        return visitorVisit(visitor, node->ifBody);
    }

    if (node->elseBody != NULL) {
        return visitorVisit(visitor, node->elseBody);
    }

    return valueInit(VALUE_NULL);
}

value_T* visitorVisitWhileLoop(visitor_T* visitor, ast_T* node) {
    value_T* result = valueInit(VALUE_NULL);

    while (true) {
        value_T* condition = visitorVisit(
            visitor,
            node->whileCondition
        );

        if (condition == (void*)0) {
            valueFree(result);
            printf("Failed to evaluate while condition\n");
            exit(1);
        }

        if (condition->type != VALUE_BOOLEAN) {
            valueFree(condition);
            valueFree(result);
            printf("While condition must evaluate to a boolean\n");
            exit(1);
        }

        bool shouldContinue = condition->booleanValue;
        valueFree(condition);

        if (!shouldContinue) {
            break;
        }

        value_T* bodyResult = visitorVisit(
            visitor,
            node->whileBody
        );

        // Preserve the most recent result without leaking the old one.
        valueFree(result);
        result = bodyResult;

        // A return inside the loop must exit the loop too.
        if (visitor->shouldReturn) {
            break;
        }
    }

    return result;
}

value_T* visitorVisitForLoop(visitor_T* visitor, ast_T* node) {
    scope_T* loopScope = node->forInitializer->scope;

    value_T* result = visitorVisit(
        visitor,
        node->forInitializer
    );
    valueFree(result);

    result = valueInit(VALUE_NULL);

    while (true) {
        value_T* condition = visitorVisit(
            visitor,
            node->forCondition
        );

        if (condition == (void*)0) {
            valueFree(result);
            printf("Failed to evaluate for condition\n");
            exit(1);
        }

        if (condition->type != VALUE_BOOLEAN) {
            valueFree(condition);
            valueFree(result);
            printf("For condition must evaluate to a boolean\n");
            exit(1);
        }

        bool shouldContinue = condition->booleanValue;
        valueFree(condition);

        if (!shouldContinue) {
            break;
        }

        value_T* bodyResult = visitorVisit(
            visitor,
            node->forBody
        );

        valueFree(result);
        result = bodyResult;

        if (visitor->shouldReturn) {
            break;
        }

        value_T* updateResult = visitorVisit(
            visitor,
            node->forUpdate
        );
        valueFree(updateResult);
    }

    // Remove the loop-local variables and free the loop scope.
    scopeRemoveVariables(loopScope, loopScope->variablesSize);

    return result;
}
