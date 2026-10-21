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

## Notes

- `E1101`–`E1122` are the parser diagnostic codes planned for the current parser migration. A code is only considered implemented once the corresponding code path calls the diagnostic reporter.
- `E1119` is reserved for unknown-namespace lookup if that error path is implemented; a namespace registration/allocation failure is not the same error.
- Allocation failures are currently reported separately and are not assigned parser error codes in this document.
- Lexer diagnostic codes are not listed here because their current definitions have not yet been verified.
- Runtime codes beyond `E2001` should be added when their diagnostic paths are confirmed.
