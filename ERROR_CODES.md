# Dara Error Codes

This document records Dara's parser and runtime diagnostic codes. Error codes are intended to remain stable as the parser is extended.

## Lexer errors

| Code | Category | Message / meaning | How to fix |
|---|---|---|---|
| `E1001` | Lexical | Unknown character. | Remove the unsupported character or replace it with valid Dara syntax. |

## Parser errors

| Code | Category | Message / meaning | How to fix |
|---|---|---|---|
| `E1101` | Syntax | Expected a particular token, but found another token. | Add or replace the token indicated by the diagnostic. |
| `E1102` | Syntax | Expected an expression. | Supply a valid expression, such as a variable, literal, function call, or parenthesized expression. |
| `E1103` | Syntax | Invalid assignment target. | Assign to a variable or a member, such as `number = 10;` or `player.health = 10;`. |
| `E1104` | Syntax | Expected an assignment operator. | Use `=`, `+=`, or `-=` where an assignment is required. |
| `E1105` | Syntax | Expected `;` or the end of a block. | Add a semicolon between statements or close the block correctly. |
| `E1106` | Syntax | Expected a function name. | Supply an identifier where a function name is required. |
| `E1107` | Syntax | Expected an identifier after `:`. | Add a valid identifier after the namespace/qualified-name separator. |
| `E1108` | Syntax | Expected a construct name. | Supply the construct's name. |
| `E1109` | Name resolution | Unknown construct. | Declare the construct or correct its qualified name. |
| `E1110` | Syntax | Expected a field or method declaration. | Add a valid field or method inside the construct body. |
| `E1111` | Type/name resolution | Unknown field type. | Use a supported type or a declared construct type. |
| `E1112` | Syntax | Expected a field name. | Add an identifier after the field type. |
| `E1113` | Syntax | Expected a field name after `.`. | Supply a member name after the dot. |
| `E1114` | Syntax | Expected a parameter type. | Supply a type in the function parameter list. |
| `E1115` | Syntax | Expected a parameter name after the type. | Add an identifier after the parameter type. |
| `E1116` | Syntax | Expected a function return type after `->`. | Supply a supported return type, such as `int`, `float`, `string`, or `noval`. |
| `E1117` | Type/name resolution | Unknown function return type. | Use a supported return type. |
| `E1118` | Syntax | Expected a namespace name. | Supply an identifier after `namespace`. |
| `E1119` | Name resolution | Unknown namespace. | Declare the namespace or correct its qualified name, if namespace lookup reports this condition. |
| `E1120` | Syntax | Expected a method name after a qualified construct. | Supply a method identifier after the construct name and dot. |
| `E1121` | Syntax | Expected a field or method name after `.`. | Supply a member identifier after the dot. |
| `E1122` | Syntax | Expected a construct name before a method call. | Call the method on a valid construct instance or use the supported qualified-construct method syntax. |

## Runtime errors

| Code | Category | Message / meaning | How to fix |
|---|---|---|---|
| `E2001` | Runtime | Cannot infer the type of a variable declared as `auto` without an initializer. | Initialize the variable, for example `auto count = 0;`, or declare an explicit type. |
| `E2002` | Runtime | Undefined variable. | Declare the variable or correct its name. |
| `E2009` | Runtime | Cannot access a field on a value that is not a construct. | Access fields on a construct instance, rather than a primitive or other non-construct value. |
| `E2010` | Runtime | Construct does not contain the requested field. | Check the construct definition and correct the field name. |
| `E2011` | Runtime — internal fallback | Unsupported or unknown AST node type. | Check the expression or statement and, if it should be supported, verify that its AST type is handled by the visitor. |
| `E2012` | Runtime | Construct nesting depth exceeds the supported limit of 64. | Reduce the depth of nested construct initialization. |
| `E2013` | Runtime | Recursive construct field type detected during default initialization. | Change the construct definitions to avoid recursive default field initialization. |
| `E2014` | Runtime | A nested construct field references an unknown construct type. | Declare the referenced construct or correct its name. |
| `E2015` | Runtime | Failed to allocate memory while creating a default construct or value. | Check available memory and ensure allocation failures are handled correctly. |
| `E2016` | Runtime | Undefined construct referenced during instantiation. | Declare the construct or correct its name. |
| `E2017` | Runtime | Duplicate field initializer in a construct instantiation. | Initialize each field no more than once. |
| `E2018` | Runtime | Field initializer has a value incompatible with its declared type. | Supply a value matching the field's declared type. |
| `E2019` | Runtime | Failed to allocate construct instance name. | Check available memory and allocation handling. |
| `E2020` | Runtime | Variable initializer has a value incompatible with the declared type. | Use an initializer matching the declared type, or change the variable's type. |
| `E2021` | Runtime | Unknown variable type. | Use a supported built-in type or declare the referenced construct. |
| `E2022` | Runtime | Compound assignment requires matching numeric types. | Use matching `int` or `float` values with `+=` or `-=`. |
| `E2023` | Runtime | Cannot assign a value of a different type to an existing variable or field. | Assign a value matching the target's existing type. |
| `E2024` | Runtime | Cannot assign a value of a different construct type. | Assign a construct instance whose type matches the target. |
| `E2025` | Runtime | Invalid assignment target encountered during evaluation. | Assign to a supported variable or construct field. |
| `E2026` | Runtime | An `if`, `while`, or `for` condition evaluated to a non-boolean value. | Make the condition evaluate to a boolean expression, such as `i < 3` or `true`. |
| `E2027` | Runtime | Arithmetic operation received operands of incompatible types or nonnumeric values. | Use matching numeric operands, such as two `int` values or two `float` values. |
| `E2028` | Runtime | Division by zero. | Ensure the divisor is not zero before performing division. |
| `E2029` | Runtime — internal fallback | Unknown arithmetic operator encountered during evaluation. | Check the parser and visitor handling for the arithmetic operator. This is an internal fallback and may not be reachable through valid Dara source code. |
| `E2030` | Runtime | Logical NOT requires a boolean operand. | Apply `!` to a boolean expression, such as `!true` or `!(number < 10)`. |
| `E2031` | Runtime — internal fallback | Unknown unary operator encountered during evaluation. | Check the parser and visitor handling for the unary operator. This is an internal fallback and may not be reachable through valid Dara source code. |
| `E2032` | Runtime | Undefined function. | Declare the function or correct its name and namespace. |
| `E2033` | Runtime | Function or method called with the wrong number of arguments. | Supply the number of arguments required by the function or method definition. |
| `E2034` | Runtime | Undefined construct referenced during method-call resolution. | Declare the construct or correct its name. |
| `E2035` | Runtime | The requested method does not exist on the specified construct. | Declare the method or correct the construct or method name. |
| `E2036` | Runtime | Reference parameter requires a variable or member-access expression. | Pass an assignable variable or construct member to the reference parameter. |
| `E2037` | Runtime | Argument type does not match the corresponding parameter type. | Pass a value matching the parameter's declared type. |
| `E2038` | Runtime | Failed to bind an argument to its parameter. | Check allocation success and ensure partially created parameter bindings are cleaned up. |
| `E2039` | Runtime | Function or method returned a value incompatible with its declared return type. | Return a value matching the declared return type, or use `noval` for functions that do not return a value. |
| `E2040` | Runtime | Failed to allocate a return value. | Check available memory and handle allocation failure correctly. |

## Array runtime diagnostics

| Code | Category | Message / meaning | How to fix |
|---|---|---|---|
| `E2003` | Runtime | Unsupported array element type. | Use `int`, `float`, `string`, `boolean`, or `auto`. |
| `E2004` | Runtime | Failed to allocate array storage or copy array data. | Check available memory and ensure allocation failures are handled correctly. |
| `E2005` | Runtime | Array method called with the wrong number of arguments. | Supply the required number of arguments for the selected method. |
| `E2006` | Runtime | An array operation received a value that is not a valid array. | Pass a properly initialized array. |
| `E2007` | Runtime | Array size or capacity exceeds the supported integer return range. | Keep the result within the supported integer range. |
| `E2008` | Runtime | Unknown `std:array` method. | Use an implemented array method. |
| `E2041` | Runtime | Array index is not an integer. | Supply an integer index. |
| `E2042` | Runtime | Array index is negative. | Supply a non-negative index. |
| `E2043` | Runtime | Array mutation requires an assignable array variable or field. | Pass an array variable or supported member-access expression. |
| `E2044` | Runtime | Value type does not match the array element type. | Supply a value compatible with the destination array's element type. |
| `E2045` | Runtime | Array capacity exceeded. | Grow the destination array or provide sufficient initial capacity. |
| `E2046` | Runtime | Array index is out of bounds. | Use an index from zero through `size - 1`. |
| `E2047` | Runtime | Invalid array range. | Ensure `start <= end <= size`. |
| `E2048` | Runtime | Array growth factor must be an integer greater than or equal to two. | Supply an integer factor of at least two. |
| `E2049` | Runtime | Array capacity must be a positive integer. | Supply a positive integer capacity when creating the array. |

Array ranges use the half-open interval `[start, end)`. The `carve` method creates a copy of the selected range, while `mend` appends copies of a source range to a destination array.

The `grow` method multiplies the existing capacity by the supplied growth factor. It does not change the array's size or automatically grow the array during insertion.

`E2002` remains reserved for undefined-variable errors and must not be used for array-capacity validation.

### Implementation consistency

The array diagnostic codes above are the intended documentation assignments, not confirmation that the current source emits each code. Before considering them implemented, align the visitor with this table:

- Index type validation should emit `E2041`.
- Negative-index validation should emit `E2042`.
- Mutable-target validation should emit `E2043`.
- Element-type mismatch should emit `E2044`.
- Capacity exhaustion should emit `E2045`.
- Index bounds errors should emit `E2046`.
- Invalid `carve` and `mend` ranges should emit `E2047`.
- An invalid `grow` factor should emit `E2048`.

The existing visitor draft uses some of these numbers for different array errors, so its diagnostic strings and codes need to be updated together. Do not change the established meanings of `E2036`–`E2040`.

Allocation failures may use `E2004` only where the corresponding allocation path is actually wired to the shared diagnostic reporter. The constructor's invalid-capacity diagnostic also needs its own unused code before it can be documented as an implemented structured diagnostic.

## Native builtin errors

Raylib builtins currently validate their argument counts and types. Some implementations report invalid arguments using direct runtime messages rather than a structured diagnostic code.

These messages are runtime failures, but they are **not assigned stable error codes in this reference yet**. Do not treat them as implemented `E20xx` diagnostics until the corresponding code paths use the shared diagnostic reporter.

Examples of currently validated calls include:

- `rl:InitWindow(width, height, title)` — expects two integers and a string.
- `rl:WindowShouldClose()` — expects no arguments.
- `rl:BeginDrawing()` — expects no arguments.
- `rl:EndDrawing()` — expects no arguments.
- `rl:CloseWindow()` — expects no arguments.
- `rl:ClearBackground(color)` — expects one color.
- `rl:DrawRectangle(x, y, width, height, color)` — expects four integers and one color.

The recognized color constants include `rl:WHITE`, `rl:BLACK`, `rl:RED`, `rl:GREEN`, and `rl:BLUE`. Passing an unsupported or incorrectly typed value to a native function may produce a runtime failure.

## Notes

- A code is considered implemented only when an actual code path emits it through the shared diagnostic system.
- `E1101`–`E1122` describe parser conditions only where the corresponding paths are wired to the diagnostic reporter; reserved codes should be marked accordingly.
- `E2001` is the runtime diagnostic for an uninitialized `auto` variable.
- `E2002` is reserved for undefined-variable failures; verify its emission path before treating it as fully implemented.
- `E2009` and `E2010` have been confirmed by runtime tests.
- `E2011` is documented for the visitor's unsupported-AST fallback; verify that path if it is exercised.
- `E2012`–`E2017` document construct initialization and instantiation errors; verify each path through the shared diagnostic reporter.
- `E2018` reports incompatible construct field initializers, and `E2019` reports failure to allocate a construct instance name. Both were added during construct-instantiation cleanup and require runtime verification.
- `E2020`–`E2025` document variable type checking and assignment errors introduced during the earlier cleanup. Verify each individual error path before considering the full set confirmed.
- `E2026` is used for non-boolean conditions in `if`, `while`, and `for` statements. Runtime emission has been confirmed. Source locations should be tested separately for these control-flow nodes.
- `E2027`, `E2028`, and `E2030` have been confirmed by runtime tests. Their reported source locations have also been verified with test output.
- `E2029` and `E2031` are internal fallback diagnostics. Their emission paths have not yet been confirmed by runtime tests.
- `E2032`–`E2040` document function and method call validation, argument binding, return-type checking, and allocation failures. Verify that the corresponding implementation paths emit these codes before considering them confirmed.
- Array diagnostics `E2003`–`E2008` remain provisional until their emission paths have been verified.
- `E2002` must not be reused for array-capacity errors while it remains assigned to undefined variables.
- Raylib builtin argument failures have not yet been assigned stable structured diagnostic codes.
- Allocation failures and other internal failures may still be reported separately from the shared diagnostic system.
- `E1001` is the lexer diagnostic for an unknown character. Its emission has been observed in runtime output; memory ownership and cleanup should also be verified with Valgrind after the diagnostic string ownership fix.
