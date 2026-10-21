# Dara Error Codes

This document records Dara's parser and runtime diagnostic codes. Error codes are intended to remain stable as the parser is extended.

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
| `E1116` | Syntax | Expected a function return type after `->`. | Supply a return type, such as `int`, `float`, `string`, or `noval`. |
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
| `E2002` | Runtime | Undefined variable. | Declare the variable or correct its name. Confirm that this error path emits `E2002` through the diagnostic reporter. |

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

- A code is considered implemented only when an actual code path emits it through the diagnostic system.
- `E1101`–`E1122` describe parser conditions only where the corresponding paths are wired to the diagnostic reporter; reserved codes should be marked accordingly.
- `E2001` is the documented runtime diagnostic for an uninitialized `auto` variable.
- Verify that undefined-variable failures emit `E2002` before treating that code as fully implemented.
- Raylib builtin argument failures have not yet been assigned stable structured diagnostic codes.
- Allocation failures and other failures may still be reported separately from the shared diagnostic system.
- Lexer diagnostic codes should be added after their current definitions and emission paths have been verified.
