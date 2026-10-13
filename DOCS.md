# Dara --- Language and Runtime Notes

This document describes the current language behavior and the
implementation model. It replaces the earlier feature-roadmap framing:
features listed as current below have been exercised in the interpreter;
ideas not implemented are explicitly marked as future work.

## 1. Interpreter pipeline

Dara is an interpreted language implemented in C:

1.  `io.c` reads the source file.
2.  `lexer.c` converts characters to tokens.
3.  `parser.c` builds AST nodes.
4.  `visitor.c` evaluates the AST.
5.  `scope.c` stores variables and function/construct definitions.
6.  `value.c` represents runtime values and manages their allocations.

The lexer recognizes syntax, the parser builds structure, the AST
retains the parsed information, scopes resolve names, values represent
runtime data, and the visitor executes the program.

## 2. Language syntax

### Declarations and types

``` dara
int amount = 10;
float speed = 30.0;
string greeting = "hello";
bool enabled = true;

func addInt(int a, int b) -> int {
    return a + b;
};
```

Parameters have an explicit type. `auto` is a generic parameter type;
`var` is accepted as an alias for `auto` in parameter declarations.
Function return types follow `->`; `noval` represents a function with no
useful return value. Construct names can be parameter types, including
qualified names where supported by the parser.

### Control flow and expressions

Implemented syntax includes `return`, `for` loops, parenthesized
expressions, arithmetic (`+`, `-`, `*`, `/`), numeric comparisons (`==`,
`!=`, `<`, `<=`, `>`, `>=`), and boolean negation (`!`). Boolean
literals are `true` and `false`. Statements are separated with
semicolons.

The language currently treats numeric runtime values as one number
representation backed by `double`; `int` and `float` are
declaration/type names rather than distinct underlying numeric storage
types. Confirm exact coercion and validation behavior before relying on
conversions.

### Constructs and fields

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

A construct declaration defines fields and methods.
`construct Name(.field = expression, ...)` creates an instance using
named initializers. Member access can be chained (`player.stats.score`).
Construct methods are declared inside the construct and called using
qualified syntax such as `Player.method(player)`; the instance is passed
explicitly rather than through implicit `this`/`self`.

### Namespaces and qualified calls

``` dara
namespace std {
    construct math {
        func addInt(int a, int b) -> int {
            return a + b;
        };
    };
};

std:math.addInt(40, 40);
```

Namespaces group declarations. A qualified path uses `:` between
namespace/construct components and `.` to select a method. For example,
the current standard-library experiment groups helpers under `std:io`,
`std:math`, and `std:string`.

### Reference parameters

A parameter type followed by `&` declares a reference parameter:

``` dara
func setScore(int& score) -> noval {
    score = 99;
};

setScore(player.stats.score);
```

A reference parameter aliases an existing assignable value. Supported
reference targets include variables and member-access expressions,
including nested fields. Temporary expressions are not valid reference
arguments. Construct references and method reference parameters are
supported as well.

Ordinary parameters use value semantics: the argument is evaluated and
stored as a separate value. Reference parameters borrow the caller's
value instead. Whole-value assignment through a reference updates the
existing referenced value in place so that the reference does not
accidentally rebind its local parameter or free caller-owned storage.

## 3. Keyword and token inventory

  -----------------------------------------------------------------------
  Word or token                       Meaning
  ----------------------------------- -----------------------------------
  `namespace`                         Namespace declaration

  `construct`                         Construct declaration or construct
                                      instantiation introducer

  `func`                              Function/method declaration

  `return`                            Return from a function

  `for`                               Loop statement

  `true`, `false`                     Boolean literals

  `int`, `float`, `string`, `bool`    Type names

  `auto`, `var`                       Generic parameter type (`var`
                                      aliases `auto` in parameter
                                      declarations)

  `noval`                             No-value return type

  `->`                                Separates a function's parameter
                                      list/body declaration from its
                                      return type

  `&`                                 Reference parameter marker

  `:`                                 Namespace/qualified-name separator

  `.`                                 Member access or named construct
                                      initializer

  `=`, `+`, `-`, `*`, `/`             Assignment and arithmetic operators

  `==`, `!=`, `<`, `<=`, `>`, `>=`    Comparisons

  `!`                                 Boolean negation

  `(`, `)`, `{`, `}`, `,`, `;`        Grouping, blocks,
                                      argument/initializer separators,
                                      statement terminator
  -----------------------------------------------------------------------

Keywords are generally recognized by the parser from identifier tokens
rather than requiring a dedicated token for every word. This table is a
language-facing inventory, not a claim that every word is a reserved
lexer token.

## 4. Runtime values and ownership

### Values

The runtime uses tagged `value_T` objects for values such as numbers,
booleans, strings, null/no-value, and construct-like structured values.
Numeric storage is currently based on `double`. Strings own their
character storage. Structured values own their field names and field
values.

### Ownership rules

These rules are important when modifying the interpreter:

-   **Owned values:** `scopeAddVariable()` stores an owned value. The
    variable's scope frees it during normal scope cleanup.
-   **Borrowed references:** `scopeAddReferenceVariable()` stores a
    non-owning alias and marks `ownsValue` false. Removing the parameter
    or freeing its scope must not free the referenced value.
-   **Copy semantics:** ordinary parameter binding evaluates/copies the
    argument into the call's local variable. Mutating that parameter
    should not mutate the caller's variable.
-   **Borrowed member access:** resolving a field returns a pointer into
    the containing construct. It is not an independent allocation and
    must not be freed separately.
-   **Replacement:** replacing an owned variable can free its old value
    and adopt the new one. Replacing a value through a reference must
    preserve the destination address, free the destination's old
    contents, move/copy the source contents into that storage, and free
    the source wrapper once.
-   **Single ownership:** each heap allocation must have one clear
    owner. Avoid freeing borrowed pointers, freeing the same allocation
    twice, or keeping pointers to values that have been freed.

`valueFree()` releases a value and its owned contents. `valueReplace()`
replaces the contents of an existing value while preserving its address;
it is used when whole-value assignment targets a reference parameter.
When changing these functions, test nested constructs, strings, copies,
and reference assignment under Valgrind.

## 5. Implementation map

  -----------------------------------------------------------------------
  File                                Responsibility
  ----------------------------------- -----------------------------------
  `io.c`                              Read source files

  `lexer.c`, `token.c`                Tokenize source and manage tokens

  `parser.c`                          Parse tokens into AST nodes

  `ast.c`                             Initialize and free AST nodes and
                                      child allocations

  `scope.c`                           Store variables, ownership flags,
                                      and definitions

  `value.c`                           Runtime value creation, copying,
                                      replacement, and cleanup

  `visitor.c`                         Evaluate AST nodes, bind arguments,
                                      resolve member access and
                                      assignments
  -----------------------------------------------------------------------

When adding a feature, update each relevant layer deliberately:
tokenization (if needed), AST representation, parser, visitor/runtime
behavior, cleanup, and tests.

## 6. Verified examples

### Generic printing and typed return values

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

        func addFloat(float a, float b) -> float {
            return a + b;
        };
    };
};

std:io.print("Hello World!");
std:io.print(std:math.addInt(40, 40));
std:io.print(std:math.addFloat(30.0, 30.0));
```

Expected output:

``` text
Hello World!
80
60
```

### References to nested fields

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

func increaseScore(int& score) -> noval {
    score = 99;
};

increaseScore(player.stats.score);
print(player.stats.score);
```

Expected output:

``` text
99
```

### Reference assignment and value-copy behavior

``` dara
construct Player {
    int x;
};

Player first = construct Player(.x = 10);
Player second = construct Player(.x = 20);

func replacePlayer(Player& p, Player other) -> noval {
    p = other;
};

replacePlayer(first, second);
print(first.x);
print(second.x);
```

The reference parameter allows `first` to be replaced in place; `second`
remains its own value.

## 7. Known limitations and future work

The following are not documented as implemented language guarantees
unless verified separately:

-   `std:string.concat()` is still a placeholder.
-   A full import/module loader and native C-library binding system are
    future work.
-   `@private` access enforcement and restrictions on external field
    mutation still need implementation/verification.
-   Do not assume recursion is safe until function-call environments are
    isolated per call.
-   String escape sequences, richer diagnostics, and full type/coercion
    rules need explicit tests/documentation.
-   A language-level `typecheck()` built-in has been discussed but
    should not be considered available until implemented.
-   The current standard-library namespace is experimental and wraps
    existing built-ins; it is not yet a complete standard library.

## 8. Development checklist

For each language change:

1.  Write one valid example and one invalid example.
2.  Add or update token and AST support as needed.
3.  Implement parsing and runtime behavior.
4.  Update AST/value/scope cleanup for any new allocations or aliases.
5.  Test nested constructs, references, and value copies when relevant.
6.  Run Valgrind after ownership changes.
