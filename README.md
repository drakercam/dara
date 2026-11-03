# Dara v0.3

Dara is an interpreted programming language written in C. It features explicit types, functions, constructs, namespaces, reference parameters, structured diagnostics, arrays, mixed numeric arithmetic, constant variables, and native Raylib bindings.

## Features

- **Types:** `int`, `float`, `string`, `bool`, `auto`, `array`, `construct` and `noval`.
- **Functions:** Typed parameters, return types, and reference parameters. Functions support the available Dara return types.
- **Constructs:** Custom data types, nested fields, designated initialization, and methods.
- **Namespaces:** Provides grouping of related data, creating qualified functions and constructs. Internal builtin functions are registered into their designated namespaces.
- **Control flow:** `if`, `while`, and `for`.
- **Operators:** Arithmetic, numeric comparisons, logical negation, assignment, and compound assignment. Mixed `int`/`float` arithmetic returns a `float`; integer-only arithmetic remains integer arithmetic.
- **Arrays:** Typed or `auto` elements, capacity-based allocation, size/capacity queries, insertion, indexed access, removal, range copying, range appending, and explicit multiplicative capacity growth.
- **Constants:** Read-only variable bindings, constant function parameters, read-only reference parameters, and protection against mutation through constant constructs and arrays.
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

### Constant variables

Prefix a variable declaration with `const` to make its binding read-only:

```dara
const int MAXHEALTH = 100;
const float MOVEMENT_SPEED = 4.5;

int health = 100;

health = 75;
MAXHEALTH = 200; // Runtime error E2050
```

A constant variable must be initialized when declared. Once declared, its binding cannot be reassigned.

Constants can use the supported Dara types, including constructs and arrays:

```dara
construct Player {
    int health;
};

const Player player = construct Player(.health = 100);

// Rejected: the construct belongs to a constant binding.
player.health = 50;

const array numbers = std:array(4) -> int;

// Rejected: the array cannot be modified through this binding.
std:array.add(numbers, 10);
```

Constant constructs cannot have their fields modified through the constant binding. Constant arrays cannot be modified through mutating array operations.

Read-only operations remain available. For example, querying an array's size or retrieving an element with `std:array.at()` does not modify the original array.

A value retrieved or copied from a constant construct or array is governed by the binding receiving that value. For example, assigning a retrieved construct to an ordinary mutable variable does not automatically make the new variable constant:

```dara
Player copy = construct Player(.health = 100);
copy.health = 50;

const Player immutableCopy = construct Player(.health = 100);
immutableCopy.health = 50; // Runtime error E2050
```

Constantness applies to the variable binding, not globally to every copy of the value.

## Mixed numeric arithmetic

Binary arithmetic accepts `int` and `float` operands in any combination. If both operands are integers, the result remains an integer. If either operand is a float, the result is a float. Numeric comparisons also accept mixed operands and return a boolean.

```dara
func main() -> noval {
    int whole = 5;
    float fraction = 2.5;

    std:print(whole + 2);        // 7, int
    std:print(whole + fraction); // 7.5, float
    std:print(fraction * 2);     // 5.0, float
    std:print(5 < fraction);     // false, boolean
    std:print(5 / 2);            // 2, integer division
    std:print(5.0 / 2);          // 2.5, floating-point division
};

main();
```

This promotion rule describes binary arithmetic and numeric comparisons; it does not mean every assignment or function-argument path implicitly converts between types. Division by zero produces `E2028`; nonnumeric operands produce `E2027`.

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

Functions can return any supported return type. Special case returns include the following types: `auto` and `construct`.

A function marked with the `auto` return type can return any supported Dara type, providing the ability to write generic functions dependent on type.

```dara
func anyReturn(int x) -> auto {
    if (x > 5) {
        return 5;
    }
    else {
        return 0.0;
    }
};

float y = anyReturn(1);
```

A function marked with the `construct` return type can return ANY construct, providing the ability to write generic construct-based functions dependent on type.

```dara
construct Player {
    int a;
};

construct Enemy {
    int b;
};

func constructReturn(int x) -> construct {
    if (x > 5) {
        return construct Player(.a = x);
    }
    else {
        return construct Enemy(.b = x);
    }
};

constructReturn(1);
```

A function marked with a specific `<construct name>` can ONLY return that specific construct type. The same rules apply to arrays in terms of storage.

```dara
construct Player {
    int a;
};

func constructReturn() -> Player {
    return construct Player(.a = 10);
};

Player player = constructReturn();
```

## Constructs and methods

Constructs define custom data types with fields and methods. Fields can contain other constructs. All specific rules for functions apply to construct methods.

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
    std:print("Running Dara v0.3");
};

main();
```

Namespaces are implemented so they can be extended whenever necessary. For example, you can define a namespace test and incorporate different methods:

```dara
namespace test {
    func returnNothing1() -> noval {
        return;
    };
};

namespace test {
    func returnNothing2() -> noval {
        return;
    };
};

test:returnNothing1();
test:returnNothing2();
```

`std:print()` is the standard print builtin.

`std:typeof()` is the standard type check builtin. It returns a string representing the target's type.

## Reference parameters

A parameter declared with `&` borrows an existing assignable value. Changes through a mutable reference affect the caller's value.

```dara
func damage(int& health) -> noval {
    health -= 10;
};

int health = 100;
damage(health);

std:print(health);
```

Ordinary parameters use copied values, while reference parameters alias existing variables or supported fields.

### Constant parameters and references

Prefix a function parameter with `const` to make its local binding read-only:

```dara
func inspect(const int value) -> noval {
    std:print(value);
    value = 50; // Runtime error E2050
};
```

A regular parameter receives a copy of the argument. A constant regular parameter therefore prevents reassignment of that local copy without affecting the caller's variable.

Combining `const` with `&` creates a read-only reference:

```dara
func inspect(const int& value) -> noval {
    std:print(value);
};
```

A constant reference can bind to either a mutable or constant variable. It can read the referenced value but cannot modify it.

A mutable reference cannot bind to a constant variable:

```dara
func modify(int& value) -> noval {
    value = 50;
};

int mutableNumber = 10;
const int immutableNumber = 20;

modify(mutableNumber);   // Allowed
modify(immutableNumber); // Runtime error E2050
```

The attempted mutable reference binding reports `E2050` and does not modify the constant.

The parameter combinations are:

| Parameter | Argument | Behavior |
|---|---|---|
| `int value` | Mutable or constant | Receives a copy |
| `const int value` | Mutable or constant | Receives a read-only copy |
| `int& value` | Mutable | Receives a mutable reference |
| `int& value` | Constant | Rejected with `E2050` |
| `const int& value` | Mutable or constant | Receives a read-only reference |

These rules also apply to construct methods because methods use the same argument-binding behavior as ordinary functions.

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

Array ranges use an inclusive start and exclusive end. Growth does not change the array's size or discard existing elements. A growth factor of `2` doubles capacity. `add` and `mend` do not automatically grow arrays; they report an error when capacity is insufficient.

Constant arrays can be queried and read, but cannot be modified through mutating operations such as `add`, `erase`, `mend`, or `grow`. These operations report runtime diagnostic `E2050` when called with a constant array binding.

Array diagnostics are documented in `ERROR_CODES.md`. Array support is still being stabilized, so validate runtime paths and memory cleanup with tests.

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

- `E2027` is reported for nonnumeric operands; mixed `int`/`float` numeric operands are valid.
- `E2028` reports division by zero.
- `E2050` reports attempts to modify a constant binding, including invalid mutable-reference bindings and mutations through constant constructs or arrays.

The C runtime manages allocated values and their owned resources. Reference parameters borrow existing storage and must not free the values they reference. Ordinary arguments use value semantics. Constant bindings enforce read-only access without changing the underlying runtime value representation.

## Current limitations

Dara is actively developed. Some standard-library operations remain experimental. Import handling, general-purpose native-library bindings, access control, and complete type/coercion rules are not all finalized. Consult the implementation and tests when behavior is not yet documented as stable.
