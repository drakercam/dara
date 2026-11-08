# Dara v0.5

Dara is an interpreted programming language written in C. It features explicit types, functions, constructs, namespaces, reference parameters, structured diagnostics, arrays, mixed numeric arithmetic, constant variables, a unified callable registry, and native Raylib bindings.

## Features

- **Types:** `int`, `float`, `string`, `bool`, `auto`, `array`, `construct` and `noval`.
- **Functions:** Typed parameters, return types, and reference parameters. Functions support the available Dara return types.
- **Callable registry:** A unified registration and lookup system for interpreted Dara functions and native C functions.
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

This builds the regular release configuration. To explicitly build either configuration:

```sh
make debug
make release
```

The debug configuration enables runtime profiling instrumentation. The release configuration builds without profiling instrumentation.

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
    std:io.print(result);
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

std:io.print(name);
std:io.print(health);
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

    std:io.print(whole + 2);        // 7, int
    std:io.print(whole + fraction); // 7.5, float
    std:io.print(fraction * 2);     // 5.0, float
    std:io.print(5 < fraction);     // false, boolean
    std:io.print(5 / 2);            // 2, integer division
    std:io.print(5.0 / 2);          // 2.5, floating-point division
};

main();
```

This promotion rule describes binary arithmetic and numeric comparisons; it does not mean every assignment or function-argument path implicitly converts between types. Division by zero produces `E2028`; nonnumeric operands produce `E2027`.

## Functions

Functions declare their return type using `->`. Use `noval` for functions that do not return a useful value.

```dara
func greet(string name) -> noval {
    std:io.print(name);
};

func multiply(int a, int b) -> int {
    return a * b;
};

greet("Dara");
std:io.print(multiply(6, 7));
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

### Function registration and the callable registry

Dara uses a unified callable registry to manage both interpreted Dara functions and native C functions.

Previously, function definitions and native functions were handled through separate registration and lookup mechanisms. The callable registry brings both implementations under one system.

Every callable is represented by a `callable_T` structure. It contains:

- The callable's name.
- A callable type identifying its implementation.
- A union containing either a Dara AST function definition or a native C callback.

The callable type is identified by `callableType_T`:

| Callable type | Implementation |
|---|---|
| `CALLABLE_DARA` | References an interpreted Dara function definition. |
| `CALLABLE_NATIVE` | References a native C function callback. |

The registry is maintained by `scope_T` and provides three primary operations:

- `scopeRegisterDaraFunction()` — registers an interpreted Dara function.
- `scopeRegisterNativeFunction()` — registers a native C function.
- `scopeGetCallable()` — searches for a callable by its qualified name.

#### Dara function registration

Dara function definitions are registered during parsing rather than waiting until runtime execution reaches their declaration.

When the parser encounters a function definition, it constructs its fully qualified name and registers the function in the root scope's callable registry.

For example:

```dara
func add(int a, int b) -> int {
    return a + b;
};
```

The function is registered under:

```text
add
```

A function inside a namespace receives a qualified name:

```dara
namespace math {
    func add(int a, int b) -> int {
        return a + b;
    };
};
```

Its registry name becomes:

```text
math:add
```

Nested namespaces extend the qualified name:

```dara
namespace game {
    namespace math {
        func add(int a, int b) -> int {
            return a + b;
        };
    };
};
```

The resulting registry name is:

```text
game:math:add
```

This allows the registry to distinguish functions with identical local names in different namespaces.

Because registration happens during parsing, functions can reference other functions declared later in the source file. Namespace-qualified references and forward function references are supported by the same mechanism.

The parser derives namespace prefixes from the existing scope hierarchy, avoiding the need to store redundant qualified-name information in every function AST node.

#### Native function registration

Native functions use the same registry but store a C callback instead of an AST function definition.

Native functions share a common callback signature:

```c
typedef value_T* (*nativeFunction_T)(
    visitor_T* visitor,
    ast_T** args,
    int argsSize
);
```

The callback receives the visitor and the original AST argument nodes. This allows native implementations to evaluate arguments through the existing visitor, preserving Dara's evaluation behavior.

For example, the standard print builtin is registered under:

```text
std:io.print
```

Raylib functions use the `rl:` namespace:

```text
rl:InitWindow
rl:BeginDrawing
rl:DrawRectangle
```

Native registration uses `scopeRegisterNativeFunction()`, while interpreted function registration uses `scopeRegisterDaraFunction()`.

Both become ordinary entries in the same callable registry.

#### Function-call resolution

When Dara evaluates a function call, the visitor uses `scopeGetCallable()` to resolve its name.

The registry searches the current scope and its parent scopes for a matching callable.

Once found, the visitor checks the callable's implementation type:

- `CALLABLE_NATIVE`: invokes the stored C callback.
- `CALLABLE_DARA`: evaluates the stored AST function definition, binds arguments, executes the function body, and handles its return value.

This means native and interpreted functions share the same function-call lookup path without requiring separate dispatch systems.

Dara function AST nodes remain owned by the program's AST tree. The callable registry stores references to those nodes rather than taking ownership of them. Native callbacks are stored as function pointers.

This separation keeps callable lookup centralized while preserving the existing ownership responsibilities of the AST and runtime.

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
        std:io.print(player.name);
        std:io.print(player.stats.health);
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
std:io.print("Hello, Dara!");

func main() -> noval {
    std:io.print("Running Dara v0.4");
};

main();
```

Namespaces can be extended across multiple declarations. Additional blocks with the same namespace name contribute declarations to the existing namespace scope.

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

Nested namespaces are also supported:

```dara
namespace game {
    namespace player {
        func update() -> noval {
            std:io.print("Updating player");
        };
    };
};

game:player:update();
```

Functions in namespaces are registered using their fully qualified names. This allows different namespaces to contain functions with identical local names.

Mutual recursion between namespace functions is supported as well:

```dara
namespace a {
    func add() -> noval {
        std:io.print("add :)");
        b:add();
    };
};

namespace b {
    func add() -> noval {
        std:io.print("add :(");
        a:add();
    };
};
```

This example demonstrates namespace-qualified mutual recursion. Since neither function has a termination condition, calling either function results in infinite recursion.

## Standard utilities (`std:util`)

The `std:util` namespace provides general-purpose runtime utilities.

### `std:util.typeof(value)`

Returns a string describing the runtime type of the supplied value.

```dara
int number = 42;
float decimal = 3.14;
string name = "Dara";
bool enabled = true;

std:io.print(std:util.typeof(number));
std:io.print(std:util.typeof(decimal));
std:io.print(std:util.typeof(name));
std:io.print(std:util.typeof(enabled));
```

`typeof` expects exactly one argument.

### `std:util.timer_begin()`

Starts or restarts the runtime performance timer.

```dara
std:util.timer_begin();
```

This function takes no arguments and does not print anything. It records the starting time using a monotonic clock, making it suitable for measuring elapsed execution time.

### `std:util.timer_end()`

Stops the active timer and prints the elapsed time in seconds and milliseconds.

```dara
std:util.timer_end();
```

This function takes no arguments. It must be called after `std:util.timer_begin()`; otherwise, a runtime diagnostic is reported.

Calling `timer_begin()` again restarts the timer from the new starting point. Calling `timer_end()` finishes the measurement and resets the timer state.

#### Benchmarking example

The timer can be used to measure the execution time of a section of Dara code:

```dara
int numIterations = 1000000;

func main() -> noval {
    std:util.timer_begin();

    int result = 0;

    for (int i = 0; i < numIterations; i += 1) {
        result += i;
    };

    std:util.timer_end();
};

main();
```

Example output:

```text
Elapsed time: 0.341272 seconds (341.272 ms)
```

The displayed time is illustrative; actual results depend on the workload and system.

**Benchmarking considerations:**

- The timer measures the code executed between `timer_begin()` and `timer_end()`.
- It measures runtime execution, not parsing or AST construction, because Dara parses the source before executing the program.
- Run benchmarks multiple times and compare repeated measurements rather than relying on a single result.
- Keep the workload identical when comparing different implementations.
- Avoid including unrelated operations, such as printing, inside the timed section unless those operations are part of what you intend to measure.

## Reference parameters

A parameter declared with `&` borrows an existing assignable value. Changes through a mutable reference affect the caller's value.

```dara
func damage(int& health) -> noval {
    health -= 10;
};

int health = 100;
damage(health);

std:io.print(health);
```

Ordinary parameters use copied values, while reference parameters alias existing variables or supported fields.

### Constant parameters and references

Prefix a function parameter with `const` to make its local binding read-only:

```dara
func inspect(const int value) -> noval {
    std:io.print(value);
    value = 50; // Runtime error E2050
};
```

A regular parameter receives a copy of the argument. A constant regular parameter therefore prevents reassignment of that local copy without affecting the caller's variable.

Combining `const` with `&` creates a read-only reference:

```dara
func inspect(const int& value) -> noval {
    std:io.print(value);
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

std:io.print(std:array.size(numbers));       // 2
std:io.print(std:array.capacity(numbers));   // 2

std:array.grow(numbers, 2);
std:io.print(std:array.capacity(numbers));   // 4

std:array.add(numbers, 30);
std:io.print(std:array.at(numbers, 2));      // 30
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

The callable registry does not own Dara function AST nodes. It stores references to definitions owned by the program AST. Native callable entries store C function pointers. This keeps function registration separate from AST and runtime-value ownership.

## Performance and profiling

Dara includes an optional runtime profiler for investigating interpreter performance and identifying expensive operations.

The profiler is enabled in the debug build and reports its results after program execution. The release build does not include profiling instrumentation.

### Profiling metrics

The profiler currently tracks several categories of runtime activity:

- **Value allocations:** Number of runtime `value_T` allocations and frees, cumulative bytes allocated, current live values, and peak live values.
- **Variable lookups:** Number of variable lookups, comparisons, scope traversals, and maximum scope depth.
- **Callable lookups:** Number of callable lookups, comparisons, scope traversals, and maximum scope depth.
- **Function dispatch:** Number of native C function calls, interpreted Dara function calls, and array method dispatches.
- **AST visits:** Number of visits to each supported AST node type.

Value allocation statistics track the outer `value_T` objects. They do not include every internal allocation made for strings, arrays, constructs, or their backing storage. Cumulative allocated bytes represent total allocation traffic, not peak memory consumption.

### Running the profiler

Build the debug configuration:

```sh
make debug
```

Run a benchmark or Dara source file:

```sh
./dara.out examples/dara_benchmarks.dara
```

To measure normal execution performance, build the release configuration:

```sh
make release
```

The debug and release configurations produce the same executable name, `dara.out`. Their object files are maintained in separate build directories to prevent profiling and non-profiling objects from being mixed.

### Benchmarking

Dara includes `examples/dara_benchmarks.dara`, which exercises several interpreter operations using a workload of one million iterations.

The benchmark currently covers:

1. Arithmetic
2. Nested expressions
3. Variable access
4. Variable assignments
5. Function calls
6. Recursive calls
7. Conditional evaluation
8. While loops
9. Construct initialization and field access
10. Array access
11. Namespace function dispatch
12. Modulus
13. Empty function calls

Use the debug build to inspect operation counts and allocation statistics. Use the release build to compare execution times without profiler instrumentation.

For meaningful comparisons:

- Keep the benchmark workload identical.
- Run benchmarks multiple times.
- Compare release timings against other release timings.
- Use profiler output to understand where runtime work is occurring.
- Avoid treating cumulative allocation bytes as actual RAM usage.

### Array access optimization

Profiling identified unnecessary deep copying as a major source of overhead in array operations.

Previously, read-only array methods evaluated their array argument through the normal visitor path. Variable evaluation returns a deep copy of the stored value. Consequently, operations such as `std:array.size()`, `std:array.capacity()`, and `std:array.at()` could copy an entire array just to inspect its size or retrieve one element.

This was particularly expensive for repeated indexed access.

The array visitor was optimized to distinguish between borrowed values and owned temporaries:

- Existing variables and member-access expressions are accessed directly through their stored values.
- Expressions that produce arrays are evaluated normally and treated as owned temporaries.
- Borrowed values are never freed by the array operation.
- Owned temporaries are released after the operation completes.

The optimization applies to the read-only operations `size`, `capacity`, `at`, and `carve`. The source argument of `mend` also uses this approach.

Value semantics are preserved:

- `at` returns an independent copy of the selected element.
- `carve` creates an independent array containing copies of the selected elements.
- `mend` appends copies of the selected source elements.
- Existing array mutation operations continue to access the actual stored array through their mutable argument resolution path.

This avoids copying an entire existing array when only a small amount of information or a subset of its elements is required.

#### Optimization results

The following results were recorded using the Dara benchmark suite and profiler:

| Metric | Before optimization | After optimization |
|---|---:|---:|
| Array access time | 4.405 s | 0.765 s |
| Value allocations | 240,006,342 | 139,001,191 |
| Value frees | 240,006,342 | 139,001,191 |
| Cumulative allocated bytes | 7.68 GB | 4.45 GB |
| Peak live values | 224 | 126 |

The array benchmark became approximately 5.75 times faster, an improvement of approximately 82.6%.

Total tracked value allocations decreased by approximately 42.1%. Peak live values also decreased, from 224 to 126.

These results demonstrate the cost of unnecessary deep copying in the interpreter and the benefits of avoiding copies when existing values can safely be borrowed.

The measurements are specific to the benchmark workload and system used. They should not be treated as universal performance guarantees.


## Current limitations

Dara is actively developed. Some standard-library operations remain experimental. Import handling, general-purpose native-library bindings, access control, and complete type/coercion rules are not all finalized. Consult the implementation and tests when behavior is not yet documented as stable.
