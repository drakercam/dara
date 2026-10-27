#include "../include/visitor.h"
#include "../include/value.h"
#include "../include/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


/*
 * Evaluates a string literal and returns a newly allocated string value.
 */
value_T* visitorVisitString(visitor_T* visitor, ast_T* node) {
	return valueInitString(node->stringValue);
}

/*
 * Evaluates a numeric literal as either an integer or a float.
 */

value_T* visitorVisitNumber(visitor_T* visitor, ast_T* node) {
	if (node->numberIsFloat) {
		return valueInitFloat(node->numberValue);
	}

	return valueInitInt((int)node->numberValue);
}

/*
 * Evaluates a boolean literal.
 */

value_T* visitorVisitBoolean(visitor_T* visitor, ast_T* node) {
	return valueInitBoolean(node->booleanValue);
}

/*
 * Evaluates statements in order and returns the result of the last
 * statement, stopping early when a return statement is encountered.
 */

value_T* visitorVisitCompound(visitor_T* visitor, ast_T* node) {
	value_T* result = (void*)0;
	
	for (size_t i = 0; i < node->compoundSize; ++i) {
		
		if (visitor->shouldReturn) {
			break;	// if the visitor visited a return statement, it stops execution and returns
		}
		
		valueFree(result);
		result = visitorVisit(visitor, node->compoundValue[i]);
	}
	
	// returns the value of its last statement, which will be useful for 
	// implementing return/expression handling
	return result;
}

/*
 * Evaluates arithmetic and comparison operations between numeric values.
 * Mixed int/float operations promote both operands to float.
 */

value_T* visitorVisitBinaryOperation(visitor_T* visitor, ast_T* node) {
    value_T* left = visitorVisit(visitor, node->binaryOperationLeft);
    value_T* right = visitorVisit(visitor, node->binaryOperationRight);

    if (!left || !right) {
        valueFree(left);
        valueFree(right);
        return (void*)0;
    }

    bool leftNumeric = left->type == VALUE_INT || left->type == VALUE_FLOAT;
    bool rightNumeric = right->type == VALUE_INT || right->type == VALUE_FLOAT;

    if (!leftNumeric || !rightNumeric) {
        visitorReportDiagnostic(
            visitor, node, "E2027",
            "Arithmetic operations require numeric operands."
        );
        valueFree(left);
        valueFree(right);
        return (void*)0;
    }

    // Convert both operands to double for comparisons and mixed arithmetic.
    double a = left->type == VALUE_INT ? left->intValue : left->floatValue;
    double b = right->type == VALUE_INT ? right->intValue : right->floatValue;

    int op = node->binaryOperationType;

    // Comparisons always return boolean.
    switch (op) {
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_NOT_EQUAL:
        case TOKEN_LESS:
        case TOKEN_MORE:
        case TOKEN_LESS_THAN_EQUAL:
        case TOKEN_MORE_THAN_EQUAL: {
            bool result;

            switch (op) {
                case TOKEN_EQUAL_EQUAL:
                    result = a == b;
                    break;

                case TOKEN_NOT_EQUAL:
                    result = a != b;
                    break;

                case TOKEN_LESS:
                    result = a < b;
                    break;

                case TOKEN_MORE:
                    result = a > b;
                    break;

                case TOKEN_LESS_THAN_EQUAL:
                    result = a <= b;
                    break;

                case TOKEN_MORE_THAN_EQUAL:
                    result = a >= b;
                    break;

                default:
                    result = false;
                    break;
            }

            valueFree(left);
            valueFree(right);

            return valueInitBoolean(result);
        }
    }

    // Preserve integer arithmetic only when both operands are integers.
    bool isInt = left->type == VALUE_INT && right->type == VALUE_INT;

    if (isInt) {
        int intA = left->intValue;
        int intB = right->intValue;
        int result;

        switch (op) {
            case TOKEN_PLUS:
            case TOKEN_PLUS_EQUAL:
                result = intA + intB;
                break;

            case TOKEN_MINUS:
            case TOKEN_MINUS_EQUAL:
                result = intA - intB;
                break;

            case TOKEN_MULTIPLY:
                result = intA * intB;
                break;

            case TOKEN_DIVIDE:
                if (intB == 0) {
                    visitorReportDiagnostic(
                        visitor, node, "E2028", "Division by zero."
                    );
                    valueFree(left);
                    valueFree(right);
                    return (void*)0;
                }

                result = intA / intB;
                break;

            default:
                visitorReportDiagnostic(
                    visitor, node, "E2029", "Unknown arithmetic operator."
                );
                valueFree(left);
                valueFree(right);
                return (void*)0;
        }

        valueFree(left);
        valueFree(right);

        return valueInitInt(result);
    }

    // If either operand is a float, return a floating-point result.
    double result;

    switch (op) {
        case TOKEN_PLUS:
        case TOKEN_PLUS_EQUAL:
            result = a + b;
            break;

        case TOKEN_MINUS:
        case TOKEN_MINUS_EQUAL:
            result = a - b;
            break;

        case TOKEN_MULTIPLY:
            result = a * b;
            break;

        case TOKEN_DIVIDE:
            if (b == 0.0) {
                visitorReportDiagnostic(
                    visitor, node, "E2028", "Division by zero."
                );
                valueFree(left);
                valueFree(right);
                return (void*)0;
            }

            result = a / b;
            break;

        default:
            visitorReportDiagnostic(
                visitor, node, "E2029", "Unknown arithmetic operator."
            );
            valueFree(left);
            valueFree(right);
            return (void*)0;
    }

    valueFree(left);
    valueFree(right);

    return valueInitFloat(result);
}

/*
 * Evaluates a unary operation on its operand.
 */

value_T* visitorVisitUnaryOperation(visitor_T* visitor, ast_T* node) {
	value_T* operand = visitorVisit(visitor, node->unaryOperationOperand);
	
	if (!operand) {
		return (void*)0;
	}
	
	switch(node->unaryOperationType) {
		case TOKEN_NOT: {
			if (operand->type != VALUE_BOOLEAN) {
				visitorReportDiagnostic(visitor, node, "E2030", "Logical NOT requires a boolean operand.");
				valueFree(operand);
				return (void*)0;
			}
			
			bool result = !operand->booleanValue;
			valueFree(operand);
			return valueInitBoolean(result);
		}
			
		default:
			visitorReportDiagnostic(visitor, node, "E2031", "Unknown unary operator.");
			valueFree(operand);
			return (void*)0;
	}
}
