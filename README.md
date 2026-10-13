# Dara

Dara is a small interpreted programming language implemented in C. It is
being developed around explicit types, constructs, functions,
namespaces, and predictable value ownership.

## Run

Build Dara using the project's build instructions, then run a source
file:

``` sh
./dara.out examples/print.dara
```

## Example

``` dara
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

Output:

``` text
Hello World!
80
```

## Language reference

### Keywords and built-in type names

The language currently uses these declaration/control words:

-   `namespace` --- groups declarations under a qualified name.
-   `construct` --- declares a construct with fields and methods.
-   `func` --- declares a function or construct method.
-   `return` --- returns a value from a function.
-   `for` --- loop statement.
-   `var` --- alias for the generic parameter type `auto` in parameter
    declarations.
-   `true`, `false` --- boolean literals.

Types used in declarations include `int`, `float`, `string`, `bool`,
`auto`, and `noval` (the no-value return type). Construct names can also
be used as types, including qualified names such as `game:Player`.

### Values, declarations, and calls

``` dara
int count = 10;
float speed = 30.0;
string name = "Dara";
bool enabled = true;

func add(int a, int b) -> int {
    return a + b;
};

int result = add(20, 22);
```

Functions declare parameter types and a return type after `->`. `auto`
accepts a value without a specific parameter type. A function that
returns no useful value declares `-> noval`.

### Constructs and namespaces

``` dara
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

Fields are initialized with named `.field = expression` entries. Fields
can be read through member access (`player.stats.score`). Construct
methods are declared with `func` inside the construct and called with
qualified syntax such as `Player.method(player)`. Namespaces use
`namespace name { ... }`; qualified names use `:` (for example,
`std:math.addInt(...)`).

### References

Append `&` to a parameter's type to bind it to an existing value instead
of making a value copy:

``` dara
func setScore(int& score) -> noval {
    score = 99;
};

setScore(player.stats.score);
```

A reference argument must be an assignable location, such as a variable
or member access---not a temporary expression. Ordinary parameters use
value semantics; modifying a copied parameter does not modify the
caller's value. Assigning a whole value through a reference updates the
referenced value rather than rebinding the local parameter.

### Operators and statements

The implemented expression syntax includes arithmetic (`+`, `-`, `*`,
`/`), numeric comparisons (`==`, `!=`, `<`, `<=`, `>`, `>=`), and
boolean negation (`!`). Parentheses can group expressions. Statements
are separated by semicolons; function and construct declarations use
braces and are commonly followed by `;`.

## Runtime and ownership

-   Variables own their stored values unless the variable is explicitly
    a reference parameter.
-   Reference parameters borrow an existing value and do not free it
    when the call's parameter scope is cleaned up.
-   Ordinary parameters receive evaluated values (currently with
    value/copy semantics).
-   Construct values contain their field values; replacing a field or
    value must free the old owned value exactly once.
-   A member-access lookup can produce a borrowed pointer into a
    construct. Borrowed values must not be freed as if they were
    independent allocations.
-   The interpreter is written in C and uses explicit
    allocation/freeing. Run tests under Valgrind when changing ownership
    or assignment behavior.

## Current scope

Dara is actively evolving. This README documents the implemented
features verified during development; it is not a promise that every
planned language feature (such as imports, native-library loading, or
private-method enforcement) is available.
