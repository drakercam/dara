# Dara — Language and Runtime Notes (v0.3)

This document describes Dara's current language behavior and implementation model. Features described as implemented reflect the current development state. Experimental behavior and future work are identified separately.

## 1. Interpreter pipeline

Dara is an interpreted programming language implemented in C.

1. `io.c` reads the source file.
2. `lexer.c` converts characters into tokens.
3. `parser.c` and the supporting parser modules build the abstract syntax tree (AST).
4. `visitor.c` evaluates the AST.
5. `scope.c` stores variables and function, construct, and namespace definitions.
6. `value.c` represents runtime values and manages their allocations.
7. `diagnostic.c` formats and prints structured diagnostics.

The lexer recognizes tokens, the parser validates syntax and constructs the AST, scopes resolve names, values represent runtime data, and the visitor executes the program.

The shared diagnostic infrastructure supports reporting a source filename, line, column, category, error code, and message. Coverage and location accuracy can vary between individual error paths.

## 2. Language syntax

### 2.1 Declarations and types

```dara
int amount = 10;
float speed = 30.0;
string greeting = "hello";
bool enabled = true;

func addInt(int a, int b) -> int {
    return a + b;
};
```

Variables can be declared with explicit types or `auto`. The `var` keyword is accepted as an alias for `auto` in supported declaration contexts.

Function parameters have explicit types. A parameter can use a built-in type or a construct type where supported. Function return types follow `->`; `noval` represents a function that returns no useful value.

**Function declarations end with a semicolon after the closing brace.**

```dara
auto inferred = 42;

func printMessage(string message) -> noval {
    std:print(message);
};

func main() -> noval {
    printMessage("Hello, Dara!");
};

main();
```

An `auto` variable needs an initializer so that its type can be inferred. Declaring an uninitialized `auto` variable produces runtime diagnostic `E2001`.

The runtime currently stores numeric values using a `double` representation. `int` and `float` are declaration/type names, not separate underlying numeric storage representations. Do not assume that all conversions or type-validation rules are complete.

### 2.2 Control flow and expressions

Implemented syntax includes:

- `if` statements.
- `while` loops.
- `for` loops.
- `return` statements.
- Parenthesized expressions.
- Arithmetic: `+`, `-`, `*`, `/`.
- Comparisons: `==`, `!=`, `<`, `<=`, `>`, `>=`.
- Boolean negation: `!`.
- Boolean literals: `true`, `false`.
- Assignment: `=`, `+=`, `-=`.
- Function calls and member access.

Statements are separated with semicolons. Blocks use braces.

```dara
func main() -> noval {
    int value = 10;
    value += 5;

    if (value > 10) {
        std:print(value);
    };
};

main();
```

Exact evaluation and coercion rules should be verified before relying on behavior outside tested examples.

### 2.3 Constructs and fields

Constructs define structured values with fields and methods.

```dara
construct Stats {
    int score;
};

construct Player {
    Stats stats;
};

Player player = construct Player(
    .stats = construct Stats(.score = 10)
);

std:print(player.stats.score);
```

A construct declaration uses `construct Name { ... };`. Instances can be created using `construct Name(...)`, with named initializers written as `.field = expression`.

Member access can be chained to access nested fields:

```dara
player.stats.score = 20;
std:print(player.stats.score);
```

Construct definitions can include methods. Methods are invoked using the supported construct-qualified method syntax. The receiver is passed explicitly rather than through an implicit `this` or `self` variable.

```dara
construct Player {
    int health;

    func report(Player& player) -> noval {
        std:print(player.health);
    };
};

Player player = construct Player(.health = 100);
Player.report(player);
```

Use syntax supported by the current parser and method-resolution implementation; do not assume every possible instance-method syntax is available.

### 2.4 Namespaces and qualified names

Namespaces group declarations under a named scope.

```dara
namespace io {
    namespace debug {
        func printMessage() -> noval {
            std:print("debug");
        };
    };
};

io:debug:printMessage();
```

Qualified names use `:` between namespace or qualified-name components. A dot is used for member access and supported method selection.

A namespace can contain functions, constructs, and nested namespaces where supported by the parser.

The standard-library namespace experiments include helpers grouped under names such as `std:math` and `std:string`. These remain experimental and should not be treated as a complete standard library. The print builtin is called directly as `std:print(...)`; examples should not substitute bare `print(...)` or assume a `std:io.print()` wrapper.

A qualified construct method call can use syntax such as:

```dara
Player.report(player);
```

The exact resolution path depends on whether the qualified name refers to a function, construct, or construct method.

### 2.5 Arrays

Dara supports array values with a fixed initial capacity, element-type information, and explicit capacity growth through the `std:array` namespace.

#### Creating arrays

```dara
array numbers;

numbers = std:array(4) -> int;
```

The optional capacity expression must evaluate to a positive integer. If no capacity is provided, the constructor uses a default capacity of one. Supported element type names include `int`, `float`, `string`, `boolean`, and `auto`.

An explicitly typed array accepts elements matching its declared type. An `auto` array accepts values of any supported runtime value type.

- **Size** is the number of elements currently stored.
- **Capacity** is the number of element slots allocated.
- **Element type** determines which values a typed array accepts.
- **Growth** increases capacity without changing the current size or existing elements.

#### Array operations

Array methods are accessed through `std:array` using the supported method-call syntax.

| Operation | Arguments | Behavior |
|---|---|---|
| `std:array.size(array)` | Array | Returns the number of stored elements. |
| `std:array.capacity(array)` | Array | Returns the allocated capacity. |
| `std:array.add(array, value)` | Array, value | Appends a copy of the value if its type is compatible and capacity is available. |
| `std:array.at(array, index)` | Array, integer index | Returns a copy of the element at the specified index. |
| `std:array.erase(array, index)` | Array, integer index | Removes the element at the index and shifts subsequent elements left. |
| `std:array.carve(array, start, end)` | Array, start index, end index | Creates a new array containing a copy of the selected range. |
| `std:array.mend(destination, source, start, end)` | Destination array, source array, start index, end index | Appends copies of a range from the source to the destination. |
| `std:array.grow(array, factor)` | Array, integer factor | Multiplies the array's capacity by the factor. |

#### Capacity growth

Growth is explicit. `add` and `mend` do not automatically resize an array when capacity is insufficient.

```dara
array numbers;
numbers = std:array(2) -> int;

std:array.add(numbers, 10);
std:array.add(numbers, 20);

std:print(std:array.size(numbers));       // 2
std:print(std:array.capacity(numbers));   // 2

std:array.grow(numbers, 2);

std:print(std:array.size(numbers));       // 2
std:print(std:array.capacity(numbers));   // 4

std:array.add(numbers, 30);

std:print(std:array.at(numbers, 2));      // 30
```

The second argument to `grow` is a multiplication factor, not an amount to add to capacity. For example, growing a capacity of 4 by a factor of 2 produces a capacity of 8. A factor of at least 2 is required. Growth preserves existing elements and does not change the array's size.

#### Array ranges

`carve` and `mend` use half-open ranges: the start index is included, and the end index is excluded. A range from index 1 to index 3 therefore selects elements at indices 1 and 2.

```dara
array numbers;
numbers = std:array(4) -> int;

std:array.add(numbers, 10);
std:array.add(numbers, 20);
std:array.add(numbers, 30);
std:array.add(numbers, 40);

array section;
section = std:array.carve(numbers, 1, 3);

std:print(std:array.size(section));       // 2
std:print(std:array.at(section, 0));      // 20
std:print(std:array.at(section, 1));      // 30

std:array.erase(numbers, 1);

std:print(std:array.at(numbers, 1));      // 30
```

`carve` creates a separate array, so changing the new array does not change the original elements. `mend` copies elements into an existing destination array and fails if the destination does not have enough capacity.

Array indices must be non-negative integers and must refer to valid elements when accessing or removing elements. Array ranges must satisfy `start <= end <= size`.

The runtime manages array storage and owned element values through the value cleanup system. Array operations and their error paths should be tested for correct ownership and cleanup, including failed allocations and nested values.

### 2.6 Native Raylib bindings

Dara exposes a small set of Raylib graphics functions through the `rl:` qualified-name prefix. These are native C builtins dispatched by the interpreter rather than functions defined in Dara source.

Implemented functions:

| Function | Purpose |
|---|---|
| `rl:InitWindow(width, height, title)` | Creates a window. |
| `rl:WindowShouldClose()` | Returns whether the window should close. |
| `rl:BeginDrawing()` | Begins a drawing frame. |
| `rl:EndDrawing()` | Finishes the drawing frame. |
| `rl:CloseWindow()` | Closes the window and releases its resources. |
| `rl:ClearBackground(color)` | Clears the frame to the specified color. |
| `rl:DrawRectangle(x, y, width, height, color)` | Draws a filled rectangle. |

The runtime also exposes these color constants:

- `rl:WHITE`
- `rl:BLACK`
- `rl:RED`
- `rl:GREEN`
- `rl:BLUE`

Color constants evaluate to the runtime's `VALUE_COLOR` type. A color stores four unsigned 8-bit components: red, green, blue, and alpha. Raylib builtins convert this value into Raylib's native `Color` structure when calling the relevant API.

Qualified constants such as `rl:WHITE` are parsed as qualified variable references. Qualified function calls such as `rl:DrawRectangle(...)` are dispatched to their native builtin implementations.

#### Raylib example

```dara
func main() -> noval {
    rl:InitWindow(1280, 720, "Dara raylib test");

    while (!rl:WindowShouldClose()) {
        rl:BeginDrawing();

        rl:ClearBackground(rl:WHITE);
        rl:DrawRectangle(100, 100, 200, 150, rl:BLUE);
        rl:DrawRectangle(400, 100, 100, 100, rl:RED);
        rl:DrawRectangle(100, 300, 400, 50, rl:GREEN);

        rl:EndDrawing();
    };

    rl:CloseWindow();
};

main();
```

Drawing operations should be performed between `rl:BeginDrawing()` and `rl:EndDrawing()`. The example creates a window, clears each frame to white, draws three colored rectangles, and closes the window when the close condition is met.

The current binding set is intentionally small. It is not a general-purpose automatic binding system for arbitrary C libraries. Additional Raylib functions must be implemented and registered with the interpreter.

Raylib availability depends on the build environment providing the Raylib development library and its required platform dependencies.

### 2.7 Reference parameters

Append `&` to a parameter's type to declare a reference parameter.

```dara
func setScore(int& score) -> noval {
    score = 99;
};

func main() -> noval {
    int score = 10;
    setScore(score);
    std:print(score);
};

main();
```

A reference parameter aliases an existing assignable value. Supported targets include variables and member-access expressions, including nested fields. Temporary expressions are not valid reference arguments.

Reference parameters are borrowed aliases, not independently owned values. Assigning through a reference updates the caller's existing value.

Ordinary parameters use value semantics: the argument is evaluated and stored as a separate value. Mutating a copied parameter should not mutate the caller's value.

Whole-value assignment through a reference must preserve the destination's address. It should replace the contents of the referenced value rather than rebinding the parameter or freeing caller-owned storage.

```dara
construct Player {
    int x;
};

Player first = construct Player(.x = 10);
Player second = construct Player(.x = 20);

func replacePlayer(Player& p, Player other) -> noval {
    p = other;
};

replacePlayer(first, second);

std:print(first.x);
std:print(second.x);
```

After the call, `first` has the replacement value, while `second` remains an independent value.

## 3. Keyword and token inventory

| Word or token | Meaning |
|---|---|
| `namespace` | Namespace declaration |
| `construct` | Construct declaration or instance creation |
| `func` | Function or method declaration |
| `return` | Return from a function |
| `if` | Conditional statement |
| `while` | While loop |
| `for` | For loop |
| `true`, `false` | Boolean literals |
| `int`, `float`, `string`, `bool` | Type names |
| `auto`, `var` | Generic/inferred type; `var` aliases `auto` in supported contexts |
| `noval` | No-value return type |
| `->` | Introduces a function return type or array element type |
| `&` | Reference parameter marker |
| `:` | Qualified-name separator |
| `.` | Member access or named initializer |
| `=`, `+=`, `-=` | Assignment operators |
| `+`, `-`, `*`, `/` | Arithmetic operators |
| `==`, `!=`, `<`, `<=`, `>`, `>=` | Comparison operators |
| `!` | Boolean negation |
| `(`, `)` | Expression grouping and argument lists |
| `{`, `}` | Blocks |
| `,` | Argument and initializer separator |
| `;` | Statement terminator |

Keywords are generally recognized by the parser from identifier tokens rather than requiring a dedicated lexer token for every word. This table describes the language-facing inventory, not the internal token representation.

## 4. Diagnostics and error reporting

Dara has a shared diagnostic representation defined by the diagnostic and source-location headers.

Parser diagnostics are reported through `parserReportError()`. The reporter creates a diagnostic with a category, code, message, and source location, then prints it using the shared diagnostic formatter.

Example output:

```text
examples/test.dara:1:4: Syntax Error[E1103]: Invalid assignment target.
```

The output identifies:

- The source file.
- The line and column.
- The diagnostic category.
- The error code.
- A human-readable explanation.

The parser uses codes in the `E11xx` range. Runtime diagnostic `E2001` reports an `auto` variable that cannot have its type inferred because it has no initializer.

See `ERROR_CODES.md` for the error-code reference. Only codes wired into actual error paths should be considered implemented. Reserved or planned codes should be labelled accordingly.

The diagnostic reporter currently uses the parser's current token location. When an error is detected after consuming an offending token, the location may point to a subsequent token; precise location tracking can be improved separately.

Allocation failures are reported separately from the language diagnostic system in some code paths.

## 5. Runtime values and ownership

### 5.1 Values

The runtime uses tagged `value_T` objects for values such as numbers, booleans, strings, arrays, no-value results, and structured construct values.

- Numeric values use `double` storage.
- Strings own their character storage.
- Construct values contain field names and field values.
- Array values contain capacity, size, element-type information, and element storage.
- Variable scopes manage the lifetime of owned variable values.
- Reference parameters borrow existing values.

### 5.2 Ownership rules

These rules are important when modifying the interpreter.

- **Owned values:** `scopeAddVariable()` stores an owned value. The owning scope is responsible for releasing it during cleanup.
- **Borrowed references:** `scopeAddReferenceVariable()` stores a non-owning alias and marks `ownsValue` false. Cleaning up the reference parameter must not free the referenced value.
- **Copy semantics:** ordinary parameter binding evaluates or copies the argument into the callee's local storage. Changes to the copy should not affect the caller's variable.
- **Borrowed member access:** resolving a field can return a pointer into a containing construct. It is not an independent allocation and must not be freed separately.
- **Replacement:** replacing an owned value may free the old value and adopt the replacement. Replacing a value through a reference must preserve the destination address and correctly transfer or copy the replacement contents.
- **Single ownership:** every heap allocation must have one clear owner. Avoid freeing borrowed pointers, freeing an allocation twice, or retaining pointers to freed values.

`valueFree()` releases a value and its owned contents. `valueReplace()` replaces the contents of an existing value while preserving its address; it is used for whole-value assignment through reference parameters.

When modifying value replacement, reference binding, or construct fields, test nested constructs, strings, ordinary copies, and reference assignment under Valgrind. Array operations should also be tested for correct element ownership and cleanup.

## 6. Implementation map

| File or component | Responsibility |
|---|---|
| `io.c` | Read source files |
| `lexer.c`, `token.c` | Tokenize source and manage tokens |
| `parser.c` and `parser_*.c` | Parse tokens into AST nodes and report syntax diagnostics |
| `ast.c` | Initialize and free AST nodes and child allocations |
| `scope.c` | Store variables, ownership flags, and definitions |
| `value.c` | Create, copy, replace, and release runtime values |
| `visitor.c` and `visitor_*.c` | Evaluate AST nodes, bind arguments, resolve member access, and perform assignments |
| `diagnostic.c` | Format and print diagnostics |
| `source.h`, `diagnostic.h` | Define source locations and diagnostic structures |

When adding a feature, update each relevant layer deliberately: tokenization if necessary, AST representation, parser, visitor/runtime behavior, memory cleanup, and tests.

## 7. Examples

### 7.1 Standard output and typed return values

```dara
func addInt(int a, int b) -> int {
    return a + b;
};

func addFloat(float a, float b) -> float {
    return a + b;
};

func main() -> noval {
    std:print("Hello World!");
    std:print(addInt(40, 40));
    std:print(addFloat(30.0, 30.0));
};

main();
```

Expected output:

```text
Hello World!
80
60
```

These examples use `std:print()` as the print builtin.

### 7.2 References to nested fields

```dara
construct Stats {
    int score;
};

construct Player {
    Stats stats;
};

Player player = construct Player(
    .stats = construct Stats(.score = 10)
);

func increaseScore(int& score) -> noval {
    score = 99;
};

func main() -> noval {
    increaseScore(player.stats.score);
    std:print(player.stats.score);
};

main();
```

Expected output:

```text
99
```

### 7.3 Reference assignment and value copies

```dara
construct Player {
    int x;
};

Player first = construct Player(.x = 10);
Player second = construct Player(.x = 20);

func replacePlayer(Player& p, Player other) -> noval {
    p = other;
};

func main() -> noval {
    replacePlayer(first, second);

    std:print(first.x);
    std:print(second.x);
};

main();
```

The reference parameter updates `first` in place. `second` remains a separate value.

## 8. Known limitations and future work

The following should not be assumed to be implemented language guarantees without separate verification:

- `std:string.concat()` is still a placeholder.
- A full import/module loader and general-purpose native C-library binding system remain future work.
- `@private` access enforcement and restrictions on external field mutation need implementation or verification.
- Recursion and function-call environment isolation need explicit testing.
- String escape sequences, richer source-location tracking, and complete type/coercion rules need explicit tests.
- A language-level `typecheck()` builtin should not be considered available until implemented.
- The standard-library namespace is experimental and is not yet a complete standard library.
- Not every parser, lexer, and runtime failure necessarily uses the shared diagnostic system yet.
- Array initialization, supported element types, and cleanup should be tested against the current implementation.

## 9. Development checklist

For each language change:

1. Write one valid example and one invalid example.
2. Add or update token and AST support as needed.
3. Implement parsing and runtime behavior.
4. Update AST, value, and scope cleanup for new allocations or aliases.
5. Test nested constructs, references, and value copies when relevant.
6. Add or update diagnostic codes and their tests.
7. Run Valgrind after ownership changes.
8. Update this document and `README.md` when language behavior changes.
