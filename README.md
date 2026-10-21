# Dara v0.1

Dara is a small interpreted programming language implemented in C. It is being developed around explicit types, constructs, functions, namespaces, reference parameters, and predictable runtime value ownership.

The project is actively evolving. This README describes the current development state, not a guarantee that every planned language feature is available.

## Features

- Typed variable declarations with `int`, `float`, `string`, and `bool`.
- `auto` declarations with type inference from an initializer.
- Functions with typed parameters and return types.
- `noval` for functions that return no useful value.
- `if`, `while`, and `for` control flow.
- Arithmetic, comparisons, boolean negation, and assignment operators.
- Constructs with fields, named initializers, and methods.
- Nested member access, such as `player.stats.score`.
- Namespaces and qualified names.
- Reference parameters using `&`, including references to nested fields.
- A shared diagnostic system being integrated across parser and runtime code.
- Runtime color values with RGBA components.
- Initial native Raylib bindings under the `rl:` namespace.
- Raylib window lifecycle functions: `rl:InitWindow`, `rl:WindowShouldClose`, and `rl:CloseWindow`.
- Frame drawing functions: `rl:BeginDrawing`, `rl:EndDrawing`, and `rl:ClearBackground`.
- Filled rectangle drawing through `rl:DrawRectangle`.
- Built-in color constants: `rl:WHITE`, `rl:BLACK`, `rl:RED`, `rl:GREEN`, and `rl:BLUE`.


## Build and run

Build the interpreter using the project's Makefile:

```sh
make
```

Run a Dara source file:

```sh
./dara.out examples/print.dara
```

If the project provides a clean target, a clean rebuild can be performed with:

```sh
make clean
make
```

## Example

```dara
namespace std {
    construct io {
        func print(auto value) -> noval {
            print(value);
        };
    };

    construct math {
        func addInt(int a, int b) -> int {
            return a + b;
        };
    };
};

std:io.print("Hello World!");
std:io.print(std:math.addInt(40, 40));
```

Expected output:

```text
Hello World!
80
```

This example assumes the standard-library wrapper definitions are available in the current interpreter configuration.

## Language overview

### Variables and functions

```dara
int count = 10;
float speed = 30.0;
string name = "Dara";
bool enabled = true;

func add(int a, int b) -> int {
    return a + b;
};

int result = add(20, 22);
```

Function parameters have explicit types. Return types follow `->`. Use `noval` for a function that returns no useful value.

`auto` infers a variable's type from its initializer. An `auto` variable without an initializer currently produces runtime diagnostic `E2001`.

`var` is accepted as an alias for `auto` in supported declaration contexts.

### Constructs and member access

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

print(player.stats.score);
```

Constructs define structured values. Instances use named initializers, and nested fields can be accessed or assigned through member expressions.

Construct methods are declared with `func` inside a construct. Supported qualified method calls pass the instance explicitly rather than relying on an implicit `this` or `self`.

### Namespaces and qualified names

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

Namespaces group declarations. The `:` separator is used in qualified names; `.` is used for member access and supported method selection.

### Reference parameters

Append `&` to a parameter's type to borrow an existing assignable value:

```dara
func setScore(int& score) -> noval {
    score = 99;
};

int score = 10;
setScore(score);
print(score);
```

Reference arguments can include supported variables and member-access expressions. They cannot be arbitrary temporary expressions.

Ordinary parameters use value semantics. Reference parameters alias existing values and must not free the referenced storage when the call's local scope is destroyed.

### Operators and statements

The current expression syntax includes:

- Arithmetic: `+`, `-`, `*`, `/`
- Comparisons: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Boolean negation: `!`
- Assignment: `=`, `+=`, `-=`

Statements are separated with semicolons. Blocks use braces.

## Graphics with Raylib

Dara includes an initial set of native Raylib bindings, allowing Dara programs to create windows and draw simple shapes.

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

The `rl:` prefix exposes the currently implemented Raylib builtins. Color constants evaluate to Dara color values, which are converted to Raylib colors by the native drawing functions.

### Raylib build requirements

The interpreter must be compiled and linked against Raylib and its required platform libraries. The project's Makefile supplies the link dependencies.

Build and run the example with:

```sh
make clean && make
./dara.out examples/test.dara
```

The example assumes that Raylib is installed and that the program is being run in an environment capable of opening a graphical window.

The binding layer is still limited. It does not automatically expose the entire Raylib API; each additional native function must be implemented and registered explicitly.


## Diagnostics

Dara's shared diagnostic infrastructure is designed to report the filename, line, column, category, code, and message.

Example:

```text
examples/test.dara:1:4: Syntax Error[E1103]: Invalid assignment target.
```

Parser errors use codes in the `E11xx` range. Runtime error `E2001` currently identifies an `auto` variable that cannot infer its type because it has no initializer.

See [`ERROR_CODES.md`](ERROR_CODES.md) for the diagnostic reference and the current status of individual error codes.

Some allocation failures and other error paths have not yet been migrated to the shared diagnostic system.

## Runtime and memory ownership

Dara is implemented in C and manages runtime memory explicitly.

- Ordinary variables own their stored values.
- Reference parameters borrow an existing value and must not free it during scope cleanup.
- Ordinary function parameters receive value copies.
- Member access can produce borrowed pointers into a containing construct.
- Replacing a value must release old owned contents exactly once while preserving the destination address when assignment occurs through a reference.

When changing reference handling, construct assignment, or value ownership, test nested constructs, strings, value copies, and reference assignment under Valgrind.

## Project documentation

- `DOCS.md` — language behavior, runtime design, ownership rules, examples, and limitations.
- `ERROR_CODES.md` — diagnostic code reference.
- `examples/` — Dara source examples and test programs.

## Current limitations

Dara is under active development. Imports, native-library loading, complete private-member enforcement, a full standard library, and comprehensive type/coercion rules should not be assumed available. Some features require additional testing before they can be treated as stable language guarantees.

Contributions and language changes should include valid and invalid examples, appropriate memory-cleanup changes, diagnostic updates, and tests.
