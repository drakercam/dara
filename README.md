# Dara v0.2

Dara is a small interpreted programming language written in C. It focuses on explicit types, structured data, functions, namespaces, reference parameters, and native graphics bindings.

Dara is actively developed, and some features remain experimental.

## Features

- **Variables and types:** `int`, `float`, `string`, `bool`, `auto`, and `var` as an alias for `auto`.
- **Functions:** Typed parameters, return types, and `noval` for functions that return no useful value.
- **Control flow:** `if`, `while`, and `for` statements.
- **Operators:** Arithmetic, comparisons, boolean negation, assignment, and compound assignment (`+=`, `-=`).
- **Constructs:** Structured values with typed fields, default initialization, named initializers, and nested member access.
- **Methods:** Construct methods with explicit instance arguments.
- **Namespaces:** Nested namespaces and qualified names using `:`.
- **References:** Reference parameters using `&`, including supported references to nested fields.
- **Arrays:** Typed array construction with capacity, element-type declarations, and size and capacity queries.
- **Runtime values:** Strings, booleans, colors, arrays, and constructs with explicit memory ownership.
- **Diagnostics:** Shared, coded parser and runtime diagnostics, with filename and source-location information.
- **Graphics:** Initial native Raylib bindings for window management, frame drawing, rectangles, and built-in colors.

## Build and run

Build Dara using the project's Makefile:

```sh
make
```

Run a source file:

```sh
./dara.out examples/print.dara
```

For a clean rebuild:

```sh
make clean
make
```

## Quick example

```dara
construct Stats {
    int score;
};

construct Player {
    string name;
    Stats stats;
};

func add(int a, int b) -> int {
    return a + b;
};

Player player = construct Player(
    .name = "Dara",
    .stats = construct Stats(.score = 10)
);

player.stats.score += add(20, 12);

print(player.name);
print(player.stats.score);
```

Expected output:

```text
Dara
42
```

## Language overview

### Variables and functions

```dara
int count = 10;
float speed = 30.0;
string name = "Dara";
bool enabled = true;

auto result = 40;
var anotherResult = 2;
```

`auto` infers its type from an initializer. A declaration such as `auto value;` cannot infer a type and currently produces runtime diagnostic `E2001`.

Functions declare parameter types and their return type using `->`:

```dara
func add(int a, int b) -> int {
    return a + b;
};

func greet(string name) -> noval {
    print(name);
};
```

### Constructs and methods

Constructs group related fields into structured values. Fields can have default values, and instances support named initialization and nested member access.

```dara
construct Player {
    int score;

    func report(Player player) -> noval {
        print(player.score);
    };
};

Player player = construct Player(.score = 100);
player.report(player);
```

Construct methods use explicit instance arguments rather than an implicit `this` or `self`.

### Namespaces

Namespaces organize declarations and support qualified names:

```dara
namespace io {
    namespace debug {
        func printMessage() -> noval {
            print("debug");
        };
    };
};

io:debug:printMessage();
```

The `:` separator identifies qualified names, while `.` is used for member access and supported method calls.

### Reference parameters

Appending `&` to a parameter type allows a function to modify an existing assignable value.

```dara
func setScore(int& score) -> noval {
    score = 99;
};

int score = 10;
setScore(score);
print(score);
```

Reference arguments can include supported variables and member-access expressions, but cannot be arbitrary temporary expressions. Ordinary parameters receive value copies; reference parameters borrow existing storage.

### Arrays

Arrays can be constructed with a capacity and declared element type:

```dara
array numbers;

numbers = std:array(5) -> int;

print(std:array:size(numbers));
print(std:array:capacity(numbers));
```

The constructor creates an array with the requested capacity and an initial size of zero. The `std:array:size` and `std:array:capacity` operations query its current size and capacity.

Array declarations, initialization, and mutation continue to evolve. Consult `DOCS.md` for current implementation details.

### Operators and statements

Supported expression operators include:

- Arithmetic: `+`, `-`, `*`, `/`
- Comparisons: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Boolean negation: `!`
- Assignment: `=`, `+=`, `-=`

Statements are separated by semicolons, and blocks use braces.

## Graphics with Raylib

Dara includes an initial set of native Raylib bindings under the `rl:` namespace.

```dara
func main() -> noval {
    rl:InitWindow(1280, 720, "Dara graphics");

    while (!rl:WindowShouldClose()) {
        rl:BeginDrawing();
        rl:ClearBackground(rl:WHITE);
        rl:DrawRectangle(100, 100, 200, 150, rl:BLUE);
        rl:EndDrawing();
    };

    rl:CloseWindow();
};

main();
```

Currently exposed functions:

- Window management: `rl:InitWindow`, `rl:WindowShouldClose`, `rl:CloseWindow`
- Drawing: `rl:BeginDrawing`, `rl:EndDrawing`, `rl:ClearBackground`, `rl:DrawRectangle`
- Colors: `rl:WHITE`, `rl:BLACK`, `rl:RED`, `rl:GREEN`, `rl:BLUE`

Raylib must be installed, and the interpreter must be linked against Raylib and its required platform libraries. The binding layer is explicit: additional functions must be implemented and registered individually.

## Diagnostics

Dara's diagnostic system reports errors with a filename, line, column, category, code, and message.

Example:

```text
examples/test.dara:1:4: Syntax Error[E1103]: Invalid assignment target.
```

Parser diagnostics use the `E11xx` range, while runtime diagnostics use the `E20xx` range.

See [`ERROR_CODES.md`](ERROR_CODES.md) for diagnostic definitions and implementation status.

## Memory management

Dara is implemented in C and explicitly manages runtime memory.

- Ordinary variables own their stored values.
- Ordinary function parameters receive value copies.
- Reference parameters borrow existing values.
- Construct fields own their stored values.
- Assignment and value replacement must release owned memory exactly once.

Memory-sensitive changes should be tested with Valgrind, especially around references, nested constructs, arrays, strings, and value copying.

## Project documentation

- [`DOCS.md`](DOCS.md) — Language behavior, runtime design, examples, and limitations.
- [`ERROR_CODES.md`](ERROR_CODES.md) — Diagnostic reference and implementation status.
- `examples/` — Example programs and test cases.

## Current limitations

Dara is under active development. Do not assume support for features that are still planned or experimental, including a complete standard library, imports, general native-library loading, comprehensive private-member enforcement, and complete type/coercion rules.

Contributions and language changes should include valid and invalid test cases, appropriate diagnostics, and correct memory cleanup.
