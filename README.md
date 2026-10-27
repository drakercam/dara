# Dara v0.2

Dara is an interpreted programming language written in C. It features explicit types, functions, constructs, namespaces, reference parameters, structured diagnostics, arrays, and native Raylib bindings.

## Features

- **Types:** `int`, `float`, `string`, `bool`, `auto`, and `noval`.
- **Functions:** Typed parameters, return types, and reference parameters.
- **Constructs:** Custom data types, nested fields, designated initialization, and methods.
- **Namespaces:** Qualified names and function calls, including `std:print()`.
- **Control flow:** `if`, `while`, and `for`.
- **Operators:** Arithmetic, comparisons, logical negation, assignment, and compound assignment.
- **Arrays:** Typed or `auto` elements, capacity-based allocation, size/capacity queries, insertion, indexed access, removal, range copying, range appending, and explicit multiplicative capacity growth.
- **Diagnostics:** Lexer, parser, and runtime errors with error codes and source locations.
- **Memory management:** Runtime value ownership, copying, reference handling, and cleanup.
- **Raylib:** Native bindings for window management, drawing, rectangles, and colors.

## Build and run

Build Dara with the included Makefile:

```sh
make
```

Run a source file:

```sh
./dara.out examples/main.dara
```

Install the interpreter:

```sh
sudo make install
```

## Quick example

```dara
func add(int a, int b) -> int {
    return a + b;
};

func main() -> noval {
    int result = add(20, 22);
    std:print(result);
};

main();
```

Output:

```text
42
```

Function declarations use the form `func name(parameters) -> returnType { ... };`. The semicolon after the closing brace is part of Dara's function declaration syntax.

## Variables and types

```dara
int health = 100;
float speed = 4.5;
string name = "Player";
bool alive = true;

auto score = 0;

std:print(name);
std:print(health);
```

Variables can use explicit types or `auto` to infer their type from an initializer. An uninitialized `auto` variable produces runtime diagnostic `E2001`.

## Functions

Functions declare their return type using `->`. Use `noval` for functions that do not return a useful value.

```dara
func greet(string name) -> noval {
    std:print(name);
};

func multiply(int a, int b) -> int {
    return a * b;
};

greet("Dara");
std:print(multiply(6, 7));
```

## Constructs and methods

Constructs define custom data types with fields and methods. Fields can contain other constructs.

```dara
construct Stats {
    int health;
    int stamina;
};

construct Player {
    string name;
    Stats stats;

    func report(Player& player) -> noval {
        std:print(player.name);
        std:print(player.stats.health);
    };
};

Player player = construct Player(
    .name = "Player",
    .stats = construct Stats(
        .health = 100,
        .stamina = 75
    )
);

Player.report(player);
```

Methods receive their object explicitly as an argument. Dara does not use an implicit `this` or `self` receiver.

## Namespaces

Namespaces organize declarations under qualified names. The `:` separator is used for namespace-qualified calls.

```dara
std:print("Hello, Dara!");

func main() -> noval {
    std:print("Running Dara v0.2");
};

main();
```

`std:print()` is the standard print builtin. Other standard-library functionality remains experimental.

## Reference parameters

A parameter declared with `&` borrows an existing assignable value. Changes through the reference affect the caller's value.

```dara
func damage(int& health) -> noval {
    health -= 10;
};

int health = 100;
damage(health);

std:print(health);
```

Ordinary parameters use copied values, while reference parameters alias existing variables or supported fields.

## Arrays

Dara arrays have a current size, an allocated capacity, and an element type. Capacity is fixed until explicitly increased with `std:array.grow()`.

```dara
array numbers;

numbers = std:array(2) -> int;

std:array.add(numbers, 10);
std:array.add(numbers, 20);

std:print(std:array.size(numbers));       // 2
std:print(std:array.capacity(numbers));   // 2

std:array.grow(numbers, 2);

std:print(std:array.capacity(numbers));   // 4

std:array.add(numbers, 30);

std:print(std:array.at(numbers, 2));      // 30
```

The constructor's capacity must be a positive integer. Supported element types include `int`, `float`, `string`, `boolean`, and `auto`. Typed arrays validate inserted values against their declared type; `auto` arrays accept any supported runtime value type.

Available operations:

- `std:array.size(array)` — returns the number of stored elements.
- `std:array.capacity(array)` — returns the allocated capacity.
- `std:array.add(array, value)` — appends a copy of a value when capacity permits.
- `std:array.at(array, index)` — returns a copy of an element.
- `std:array.erase(array, index)` — removes an element and shifts subsequent elements.
- `std:array.carve(array, start, end)` — creates a new array from a range.
- `std:array.mend(destination, source, start, end)` — appends a range from another array.
- `std:array.grow(array, factor)` — multiplies the capacity by the supplied factor.

Array ranges use an inclusive start and exclusive end. For example, `std:array.carve(numbers, 1, 3)` selects indices 1 and 2.

Growth does not change the array's size or discard its existing elements. A growth factor of `2` doubles the capacity: 4 becomes 8, then 16 on the next doubling. `add` and `mend` do not automatically grow arrays; they report an error when the available capacity is insufficient.

Array diagnostics are documented in `ERROR_CODES.md`. Array support is still being stabilized, so validate the relevant runtime paths and memory cleanup with tests before relying on undocumented behavior.

## Raylib

Dara provides native bindings for a small set of Raylib functions:

- `rl:InitWindow`
- `rl:WindowShouldClose`
- `rl:BeginDrawing`
- `rl:EndDrawing`
- `rl:CloseWindow`
- `rl:ClearBackground`
- `rl:DrawRectangle`

Available color constants include `rl:WHITE`, `rl:BLACK`, `rl:RED`, `rl:GREEN`, and `rl:BLUE`.

```dara
func main() -> noval {
    rl:InitWindow(800, 600, "Dara");

    while (!rl:WindowShouldClose()) {
        rl:BeginDrawing();
        rl:ClearBackground(rl:BLACK);
        rl:DrawRectangle(350, 250, 100, 100, rl:RED);
        rl:EndDrawing();
    };

    rl:CloseWindow();
};

main();
```

Raylib must be installed and available to the native bindings used by the interpreter.

## Diagnostics and memory management

Dara reports language errors using diagnostic categories, error codes, source locations, and explanatory messages. See `ERROR_CODES.md` for the diagnostic reference.

The C runtime manages allocated values and their owned resources. Reference parameters borrow existing storage and must not free the values they reference. Ordinary arguments use value semantics.

## Documentation

See `Language and Runtime Notes` for the interpreter architecture, language syntax, runtime ownership rules, diagnostics, and implementation details.

## Current limitations

Dara is actively developed. Some standard-library operations remain experimental. Import handling, general-purpose native-library bindings, access control, and complete type/coercion rules are not all finalized. Consult the implementation and tests when behavior is not yet documented as stable.
