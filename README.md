# Dara v0.2

Dara is an interpreted programming language written in C. It provides explicit types, constructs, functions, namespaces, references, runtime diagnostics, and native Raylib bindings.

## Features

- **Variables and types:** `int`, `float`, `string`, `bool`, `auto`, and `noval`.
- **Functions:** Parameters, return values, and explicit return types.
- **Constructs:** Custom data types, nested fields, designated initialization, and methods.
- **Namespaces:** Qualified function and method calls, such as `std:print()`.
- **References:** Reference parameters using `&`, allowing functions to modify existing variables and fields.
- **Control flow:** `if`, `else`, `while`, and `for`.
- **Operators:** Arithmetic, comparison, logical, assignment, and compound-assignment operators.
- **Arrays:** Typed arrays, capacity allocation, element storage, and size/capacity queries.
- **Runtime diagnostics:** Categorized lexer, parser, and runtime errors with error codes and source locations.
- **Memory management:** Explicit allocation and cleanup within the C runtime, including value copying and reference ownership.
- **Raylib bindings:** Window creation, drawing, frame handling, rectangles, and predefined colors.

## Build and run

Build Dara using the included Makefile:

```sh
make
```

Run a Dara source file:

```sh
./dara.out examples/main.dara
```

Install the interpreter:

```sh
sudo make install
```

## Example

```dara
int number = 42;
float decimal = 3.14;
string message = "Hello, Dara!";

std:print(message);
std:print(number);
std:print(decimal);
```

## Language overview

### Variables and types

Declare variables with an explicit type or use `auto` to infer their type from an initializer.

```dara
int health = 100;
float speed = 4.5;
string name = "Player";
bool alive = true;

auto score = 0;
```

Typed variables can be declared without an initializer where supported. An `auto` variable requires an initializer because its type cannot otherwise be inferred.

### Functions

Functions can accept parameters and return values. Use `noval` for functions that do not return a value.

```dara
int add(int a, int b) {
    return a + b;
}

noval greet(string name) {
    std:print(name);
}

std:print(add(10, 20));
greet("Dara");
```

### Constructs and methods

Constructs define custom data types with named fields and methods. Fields can contain other constructs, allowing nested data structures.

```dara
construct Stats {
    int health;
    int stamina;
}

construct Player {
    string name;
    Stats stats;

    noval report(Player& player) {
        std:print(player.name);
        std:print(player.stats.health);
    }
}

Player player = {
    .name = "Player",
    .stats = {
        .health = 100,
        .stamina = 75
    }
};

player.report(player);
```

Methods use an explicit receiver argument rather than an implicit `this`. References allow the receiver or other arguments to be modified without copying their underlying value.

### Namespaces

Namespaces organize functions and support qualified calls.

```dara
std:print("Hello!");

std:math:addInt(10, 20);
```

Namespace-qualified calls can also be used to organize native library bindings.

### References

Use `&` in a parameter declaration to receive a reference to an existing variable or field.

```dara
noval damage(int& health) {
    health -= 10;
}

int health = 100;
damage(health);

std:print(health);
```

Reference arguments must refer to assignable variables or fields. Ordinary parameters receive copied values.

### Operators and control flow

Dara supports arithmetic, comparisons, logical operations, assignment, and compound assignment.

```dara
int counter = 0;

while (counter < 5) {
    std:print(counter);
    counter += 1;
}

if (counter == 5) {
    std:print("Finished");
}
```

`for` loops are also supported.

### Arrays

Arrays support allocated capacity, element storage, and size/capacity queries through the standard array namespace.

```dara
array numbers = std:array(5) -> int;

std:print(std:array:size(numbers));
std:print(std:array:capacity(numbers));
```

An array's size represents its current number of stored elements; capacity represents its allocated storage. Array operations and supported element types depend on the current runtime implementation.

## Raylib

Dara provides native bindings for basic Raylib functionality, including:

- `rl:InitWindow`
- `rl:WindowShouldClose`
- `rl:BeginDrawing`
- `rl:EndDrawing`
- `rl:CloseWindow`
- `rl:ClearBackground`
- `rl:DrawRectangle`

Predefined colors include `WHITE`, `BLACK`, `RED`, `GREEN`, and `BLUE`.

Example:

```dara
define raylib as rl;

rl:InitWindow(800, 600, "Dara");

while (!rl:WindowShouldClose()) {
    rl:BeginDrawing();
    rl:ClearBackground(BLACK);
    rl:DrawRectangle(350, 250, 100, 100, RED);
    rl:EndDrawing();
}

rl:CloseWindow();
```

Raylib must be installed and available to the native bindings used by the interpreter.

## Diagnostics and memory management

Dara reports lexer, parser, and runtime errors with diagnostic codes, source locations, and descriptive messages.

The interpreter manages runtime values and their allocated resources internally. Ordinary Dara code does not directly manage C allocations. Reference parameters borrow existing values, while ordinary arguments use copied values.

## Project documentation

Consult the project's documentation for further details about:

- Language syntax and runtime behavior
- Diagnostic and error codes
- Memory ownership and value handling
- Implemented features and planned development

## Current scope

Dara is actively developed. Some features and standard-library operations remain incomplete or experimental. Import handling, native-library integration, access control, and broader standard-library functionality continue to evolve.

This README describes the intended usage of currently implemented features; consult the source and detailed documentation for exact behavior where support is still evolving.
