# Dara Language --- Codebase Guide and Feature Implementation Roadmap

> This document describes the C implementation shown in the supplied
> source snippets and maps the proposed Dara language features to the
> functions and files that would need to change.
>
> **Scope note:** The snippets include implementation files but not the
> corresponding headers (`include/*.h`) or the program entry point
> (`main.c` or equivalent). The lexer source was pasted twice. Names and
> responsibilities below are based on the code shown; confirm
> declarations and the actual entry point in the project before making
> changes.

## 1. The big picture

Dara currently follows a conventional small-interpreter pipeline:

1.  **Read source text** into memory.
2.  **Lex** the text into tokens.
3.  **Parse** tokens into an abstract syntax tree (AST).
4.  **Visit/evaluate** the AST to execute the program.
5.  **Store runtime state** in scopes, variables, function definitions,
    and runtime values.
6.  **Free** the allocations owned by each layer.

A useful mental model is:

``` text
source file
    |
    v
fileGetContents()             io.c
    |
    v
lexerGetNextToken()           lexer.c
    |
    v
parserParse()                 parser.c
    |
    v
AST nodes                     ast.c / ast.h
    |
    v
visitorVisit()                visitor.c
    |
    +---- scopes               scope.c
    |
    +---- runtime values       value.c
    |
    v
program effects (print, etc.)
```

The lexer does not understand program meaning. It recognizes characters
and creates tokens. The parser does not execute code; it arranges tokens
into AST nodes. The visitor executes those nodes and uses scopes and
values to represent runtime state.

## 2. Files and functions

### `io.c` --- source-file input

#### `fileGetContents(const char *filePath)`

-   Opens a source file in binary mode.
-   Seeks to determine its length.
-   Allocates a buffer, reads the contents, adds a terminating NUL byte,
    closes the file, and returns the buffer.
-   If opening the file fails, prints an error and exits.

**Ownership:** The returned buffer is dynamically allocated. In the
shown implementation, `lexerFree()` frees `lexer->contents`, so the
source buffer is effectively transferred to the lexer after
`lexerInit(contents)`.

**Things to revisit later:** check `fseek`, `ftell`, `fread`, and
allocation failures before using their results. If `calloc` fails,
`buffer[length]` would dereference a null pointer.

### `lexer.c` --- characters to tokens

#### `lexerInit(char *contents)`

Creates a lexer, stores the source buffer, and initializes its
index/current character.

#### `lexerFree(lexer_T *lexer)`

Frees the source buffer and lexer. This means the lexer owns the
`contents` pointer passed to `lexerInit()` in the current design.

#### `lexerAdvance(lexer_T *lexer)`

Moves the index forward and updates the current character.

#### `lexerSkipWhitespace(lexer_T *lexer)`

Advances through whitespace using `isspace()`. This includes tabs and
newlines.

#### `lexerGetNextToken(lexer_T *lexer)`

The main token-dispatch function. It examines the current character and
chooses a collector or punctuation/operator case.

Currently recognized categories include: - integer-looking numeric text
(`TOKEN_NUMBER`) - alphanumeric identifiers (`TOKEN_ID`) - strings in
double quotes (`TOKEN_STRING`) - `=`, `==`, `;`, parentheses, braces,
comma - `+`, `-`, `*`, `/` - `<`, `<=`, `>`, `>=`, `!`, `!=` - end of
input (`TOKEN_EOF`)

Unknown characters are printed and skipped.

This is the first place to modify when a new character-level operator or
punctuation mark is introduced. Examples: `->`, `.`, `:`, `@`, `&&`, and
`||`.

#### `lexerCollectString(lexer_T *lexer)`

Consumes the opening quote, copies characters until the next quote,
consumes the closing quote, and returns a string token.

#### `lexerCollectNumber(lexer_T *lexer)`

Collects consecutive digits and returns a number token. The current
collector does **not** recognize decimal points, signs, or exponents.
Although the parser converts token text using `strtod`, the lexer as
shown only collects digits.

#### `lexerCollectID(lexer_T *lexer)`

Collects alphanumeric characters and returns an identifier token. The
current loop allows letters and digits, but not underscores. It starts
only when the current character is alphabetic, so names such as
`_internal` are not currently supported.

#### `lexerAdvanceWithToken(lexer_T *lexer, token_T *token)`

Advances once and returns the supplied token. Used for single-character
operators/punctuation.

#### `lexerGetCurrCharAsStr(lexer_T *lexer)`

Allocates a two-byte C string containing the current character and a
terminating NUL byte. Callers must free the temporary string after
copying it; the shown collectors do so.

**Lexer extension rule:** introduce a token type in `token.h`, teach
`lexerGetNextToken()` to recognize it, then teach the parser which
grammar production consumes it. A token existing by itself does not make
a feature work.

### `token.c` --- token storage

#### `tokenInit(int type, char *value)`

Allocates a token and stores its type and owned string value.

#### `tokenFree(token_T *token)`

Frees the token's value and the token itself.

Tokens are temporary parser input. AST nodes generally copy the text
they need before the parser advances and frees old tokens.

### `parser.c` --- tokens to AST

#### `parserInit(lexer_T *lexer)`

Creates a parser, fetches its first token, clears the previous-token
pointer, and creates a scope.

#### `parserFree(parser_T *parser)`

Frees the previous/current tokens, parser scope, and parser. Check the
full program's cleanup order: AST nodes and runtime scopes may retain
pointers to scopes or function ASTs, so ownership must be consistent.

#### `parserEat(parser_T *parser, int tokenType)`

Consumes the current token only if its type matches the expected type.
It frees the older previous token, moves current to previous, and
fetches the next token. Otherwise, it prints an error and exits.

This function is the parser's basic "expect this token" operation. It is
not the place to implement a whole feature, but nearly every parser
production uses it.

#### `parserParse(parser_T *parser, scope_T *scope)`

Entry point for parsing a program; delegates to
`parserParseStatements()`.

#### `parserParseStatement(parser_T *parser, scope_T *scope)`

Dispatches a statement based on the current token. Currently identifiers
are routed to `parserParseID()`; other tokens become `AST_NOOP`.

As new statement forms are added, ensure they are reachable through this
dispatch path. Since keywords are currently lexed as `TOKEN_ID`, keyword
recognition occurs in `parserParseID()`.

#### `parserParseStatements(parser_T *parser, scope_T *scope)`

Builds an `AST_COMPOUND` containing a sequence of statements separated
by semicolons. It assigns the same scope pointer to the compound and its
first statement.

This is central to blocks and function bodies. Constructs, functions,
`invoke main`, and loops will all depend on correct block termination
and statement parsing.

**Important current grammar detail:** function definitions in the
examples end with `};`. The function parser consumes the closing `}`,
while the enclosing statement sequence consumes the semicolon.

#### `parserParseExpression(parser_T *parser, scope_T *scope)`

Parses `+` and `-` expressions by combining terms into
`AST_BINARY_OPERATION` nodes.

#### `parserParseTerm(parser_T *parser, scope_T *scope)`

Parses `*` and `/` with higher precedence than addition/subtraction by
combining unary expressions into binary-operation nodes.

#### `parserParseUnary(parser_T *parser, scope_T *scope)`

Parses prefix `!` recursively, producing `AST_UNARY_OPERATION`;
otherwise delegates to `parserParseFactor()`.

#### `parserParseFactor(parser_T *parser, scope_T *scope)`

Parses primary expressions: strings, numbers, booleans,
identifiers/variable references/function calls, and parenthesized
expressions. This is a major extension point for new expression syntax
such as construct literals (`Player{...}`), field access (`p.x`),
qualified names (`rl:RED`), and imported module references.

#### `parserParseComparison(parser_T *parser, scope_T *scope)`

Parses comparisons (`==`, `<`, `>`, `<=`, `>=`, `!=`) around expressions
and creates binary-operation AST nodes.

For logical operators, add a separate precedence layer rather than
simply putting `&&` and `||` into this loop. A common precedence order
is: comparison, logical NOT, logical AND, logical OR, with arithmetic
tighter than comparisons. The exact parser function arrangement should
reflect the intended precedence.

#### `parserParseFunctionCall(parser_T *parser, scope_T *scope)`

Uses the previous token as the function name, consumes parentheses,
parses zero or more comma-separated argument expressions, and creates
`AST_FUNCTION_CALL`.

This currently supports calls such as `hello(1, 2)`, not yet qualified
calls such as `Player.update(p)` or `rl:DrawRectangle(...)`.

#### `parserParseVariable(parser_T *parser, scope_T *scope)`

Consumes an identifier. If followed by `(`, it parses a function call;
otherwise, it creates an `AST_VARIABLE`.

It will need a deliberate redesign or a new expression parser for dotted
field access and namespace-qualified names. Avoid trying to handle all
of these by adding more ad-hoc string checks here.

#### `parserParseVariableDefinition(parser_T *parser, scope_T *scope)`

Parses `var name = expression` and creates `AST_VARIABLE_DEFINITION`.

To support `var x -> float` and `var game -> Game = expression`, this
production will need to parse an optional type annotation and store it
in the AST. It must also allow declarations without initializers if
construct fields are declared as `var x -> float;`.

#### `parserParseReturnStatement(parser_T *parser, scope_T *scope)`

Consumes `return`, optionally parses a return expression, and creates
`AST_RETURN`. It recognizes a bare return when the next token is a
semicolon or right brace.

#### `parserParseFunctionDefinition(parser_T *parser, scope_T *scope)`

Parses `func name(parameters) { body }` into `AST_FUNCTION_DEFINITION`.
It supports zero parameters. It parses the body using
`parserParseStatements()` and records the surrounding scope.

For constructs, the parser must distinguish ordinary function
definitions from construct member functions and retain the owning
construct's name/metadata.

#### `parserParseString(parser_T *parser, scope_T *scope)`

Copies the current string token into an `AST_STRING`.

#### `parserParseNumber(parser_T *parser, scope_T *scope)`

Converts the current numeric token text to a `double` and creates an
`AST_NUMBER`.

#### `parserParseID(parser_T *parser, scope_T *scope)`

Dispatches identifier keywords: `var`, `func`, and `return`; otherwise
parses a variable reference or function call. Most new keyword-led
statements (`construct`, `define`, `import`, `from`, `invoke`, `while`)
need a route through this function or through a more explicit
keyword/token dispatch design.

### `ast.c` --- AST node creation and destruction

#### `astInit(int type)`

Allocates and zero-initializes an AST node, sets its type, and
initializes the current node-specific pointers and sizes.

#### `astFree(ast_T *ast)`

Recursively frees AST-owned strings, child nodes, and arrays according
to node type. Every new AST node kind must be added here if it owns
child nodes or allocations.

#### `astFreeVariableDefinition(ast_T *ast)`

A specialized cleanup function that frees a variable definition's name
and AST node. It does not free the initializer child in the shown
version, so be careful using it; the general `astFree()` handles that
child.

**AST design rule:** the parser describes structure; the AST must retain
everything needed at runtime. For example, a typed field needs its name
and declared type, a construct definition needs its fields and methods,
and a module import needs the module/alias information.

### `value.c` --- runtime values and structured data

#### `valueInit(int type)`

Allocates a zero-initialized runtime value of the requested type.

#### `valueFree(value_T *value)`

Frees the value and any owned nested data. For `VALUE_STRING`, it frees
the string. For `VALUE_STRUCT`, it frees each field name, each field
value, and the field array.

This is the natural foundation for construct instances, since
`VALUE_STRUCT` already represents named fields.

#### `valueInitNumber(double number)`

Creates a numeric runtime value.

#### `valueInitString(const char *string)`

Creates a string runtime value with its own copy of the text.

#### `valueInitBoolean(bool boolean)`

Creates a boolean runtime value.

#### `valueInitStruct(void)`

Creates an empty struct value with no fields.

#### `valueStructSetField(value_T *value, const char *name, value_T *fieldValue)`

Adds a named field to a struct or replaces an existing field.
Replacement frees the previous field value, then takes ownership of the
new value.

#### `valueStructGetField(value_T *value, const char *name)`

Looks up a named field and returns its stored value pointer, or NULL if
the input is not a struct or the field is missing.

**Ownership warning:** `valueStructGetField()` returns a borrowed
pointer into the parent struct. Do not blindly `valueFree()` that
pointer, or the parent will later try to free it again. When an
expression needs an independent result, use `valueCopy()`.

#### `valueCopy(const value_T *value)`

Deep-copies null, number, boolean, string, and struct values. This is
important for the current value semantics: `visitorVisitVariable()`
returns a copy of a variable's value, so modifying that returned copy
does not mutate the original variable automatically.

That behavior directly affects your proposed `p.x += 1` syntax. Field
assignment must explicitly locate and update the field inside the
original stored struct value, rather than modifying a temporary copy
produced by ordinary variable lookup.

### `scope.c` --- variable and function environments

#### `scopeInit(void)`

Creates an empty scope with no variables or function definitions.

#### `scopeFree(scope_T *scope)`

Frees the function-definition pointer array, each variable and its
value, the variable array, and the scope. The function ASTs themselves
are not freed by this function; their ownership needs to be handled
elsewhere.

#### `scopeAddFunctionDefinition(scope_T *scope, ast_T *funcDef)`

Adds a pointer to a function-definition AST to the scope. The scope
stores the pointer, not a deep copy of the AST.

#### `scopeGetFunctionDefinition(scope_T *scope, const char *funcName)`

Searches the scope's function list by name and returns the matching AST
pointer.

#### `scopeAddVariable(scope_T *scope, const char *name, value_T *value)`

Creates a variable entry and stores the supplied runtime value pointer.
In the current design, the scope owns that value.

#### `scopeGetVariable(scope_T *scope, const char *name)`

Searches variables from newest to oldest, so a recently added variable
with the same name shadows an earlier one in that scope.

#### `scopeRemoveVariables(scope_T *scope, size_t count)`

Removes the most recently added variables and frees their values. The
function call implementation uses this to remove arguments after a call.

**Important design limitation:** function parameters are currently
inserted into the function definition body's stored scope. That makes
call-local state share a scope between calls and is unsafe for recursion
or concurrent/re-entrant calls. Before building a larger language,
introduce a per-call environment/frame that points to a parent
environment, or another equivalent local-scope mechanism.

### `visitor.c` --- AST evaluation

#### `visitorInit(void)` / `visitorFree(visitor_T *visitor)`

Allocate/free the visitor. The visitor currently also tracks
`shouldReturn`, which controls early exit from a compound body.

#### `visitorVisit(visitor_T *visitor, ast_T *node)`

Central AST dispatcher. It calls the specialized visitor function for
each AST node type. When a new AST node is introduced, add a case here
as well as implementing its evaluator.

#### `visitorVisitVariableDefinition(visitor_T *visitor, ast_T *node)`

Evaluates the initializer, then adds the resulting value to the node's
scope under the declared name. The scope takes ownership of that value.

#### `visitorVisitFunctionDefinition(visitor_T *visitor, ast_T *node)`

Registers the function AST in the node's scope. It does not execute the
function body at definition time.

#### `visitorVisitVariable(visitor_T *visitor, ast_T *node)`

Looks up a variable and returns a deep copy of its value. If missing,
reports an undefined variable and exits.

#### `visitorVisitFunctionCall(visitor_T *visitor, ast_T *node)`

Handles built-in `print` specially. For other names, it: 1. Finds a
function definition in the call node's scope. 2. Checks the argument
count. 3. Evaluates each argument. 4. Adds argument values as parameter
variables to the function body's scope. 5. Executes the body. 6.
Converts a function that did not explicitly return into `VALUE_NULL`. 7.
Removes the parameter variables and returns the result.

This is the core function-call path, and it will eventually need to
distinguish ordinary functions from construct methods, imported native
functions, and qualified names. It also needs per-call local scopes to
support recursion reliably.

#### `visitorVisitString(visitor_T *visitor, ast_T *node)`

Creates a runtime string from a string AST.

#### `visitorVisitNumber(visitor_T *visitor, ast_T *node)`

Creates a runtime number from a number AST.

#### `visitorVisitBoolean(visitor_T *visitor, ast_T *node)`

Creates a runtime boolean from a boolean AST.

#### `visitorVisitCompound(visitor_T *visitor, ast_T *node)`

Executes statements in order and stops when `visitor->shouldReturn`
becomes true. It returns the last statement's result. Return propagation
currently relies on the visitor-wide flag, which is simple for early
implementation but can become fragile with nested calls; return state
should eventually belong to each function-call frame.

#### `visitorVisitBinaryOperation(visitor_T *visitor, ast_T *node)`

Evaluates both operands and implements arithmetic and numeric
comparisons. It validates numeric operand types, handles division by
zero, frees operand values, and returns a numeric or boolean result.

For field assignment, this evaluator alone is not sufficient: assignment
is a statement/expression with a target that must be resolved as a
mutable storage location. Do not implement `p.x += 1` by evaluating
`p.x` into a copy and changing only that copy.

#### `visitorVisitUnaryOperation(visitor_T *visitor, ast_T *node)`

Evaluates prefix `!`, requires a boolean, and returns its negation.

#### `visitorVisitReturn(visitor_T *visitor, ast_T *node)`

Evaluates the optional return expression (or creates `VALUE_NULL`) and
marks `shouldReturn = true`. The compound visitor observes this flag and
stops executing later statements.

#### Built-ins in `visitor.c`

-   `builtinFunctionPrint(...)`: evaluates each argument and prints its
    value. Current behavior prints one argument per line and prints
    `noval` for null.
-   `builtinFunctionAdd(...)`: currently incomplete as a user-facing
    arithmetic function; it accumulates numeric arguments into an `int`
    but does not return the computed result, and its `default` branch is
    missing a `break`/error-safe structure.
-   `builtinFunctionSub(...)`, `builtinFunctionMultiply(...)`,
    `builtinFunctionDivide(...)`: declared but empty in the shown
    source. They need real implementations or should be removed until
    supported.

## 3. The key architectural decisions for Dara

### 3.1 Keep definitions separate from instances

Your `construct Player` idea is best represented by two distinct runtime
concepts:

-   **Construct definition:** blueprint metadata---name, declared
    fields, field types, method definitions, annotations.
-   **Construct instance:** a `VALUE_STRUCT` containing actual field
    values.

For example:

``` dara
construct Player {
    var x -> float;
    var y -> float;

    func update(p) {
        p.x += 1;
        p.y += 1;
    };
};

var p = Player{.x = 1, .y = 1};
```

`Player` is the blueprint. `p` is the data instance. The method
`Player.update(p)` looks up a method in the `Player` blueprint and
operates on the supplied instance.

The existing `VALUE_STRUCT` is a useful starting point for the instance.
It is not, by itself, enough to represent the blueprint or enforce
privacy and declared field types.

### 3.2 Decide value semantics before implementing field mutation

Today, variable reads return deep copies. This is easy to reason about
for numbers and strings, but it means this naïve evaluation sequence
will not mutate the original:

``` text
evaluate p      -> deep copy of struct
read .x         -> value in that copy
add 1           -> new number
```

Instead, assignment should be evaluated as a target operation:

1.  Resolve the variable `p` to its stored `value_T *`.
2.  Resolve field `x` inside that stored struct.
3.  Check whether the current execution context is allowed to mutate it.
4.  Evaluate the right-hand side.
5.  Replace the stored field value or update it according to the
    operator.
6.  Free any replaced value exactly once.

A good long-term interface is to separate **read** from
**resolve-for-write**, for example: - `visitorResolveLValue(...)`
returns a storage location for an assignment target. -
`visitorVisit(...)` evaluates an expression and returns a value. -
`valueStructGetField(...)` reads a field. - `valueStructSetField(...)`
writes/replaces a field.

Do not expose a raw mutable field pointer everywhere; that would make
access rules difficult to enforce.

### 3.3 Represent assignment explicitly

The current parser supports `var name = expression`, but not general
assignment statements such as `x = 2`, `p.x = 2`, or `p.x += 1`.

Introduce AST nodes for assignment (or a carefully designed assignment
AST that includes a target, operator, and value). The target should
support: - variable target: `x` - field target: `p.x` - eventually
indexed target: `items[i]`

For compound assignment, `p.x += 1`, evaluate the target location once,
read its current value, evaluate the right-hand side, apply the
operator, then write the result back. This avoids resolving
side-effecting targets twice when indexing or function calls are added
later.

### 3.4 Use per-call environments

The current implementation places function parameters into the function
body's stored scope, then removes them afterward. This is fragile
because the function definition's AST is reused for each call.

A cleaner model: - Each function call creates a new local scope/frame. -
Parameters are inserted into that call frame. - The frame points to a
parent scope for globals and module definitions. - The function body
executes with the call frame as its environment. - The frame is
destroyed when the call returns.

This should be addressed before recursion, complex nested calls, or game
code with lots of repeated method calls.

### 3.5 Keep parsing, validation, and runtime behavior distinct

For a feature like typed construct fields: - Lexer: recognize `->`. -
Parser: store the declared type in the AST. - Definition/validation
stage: resolve type names and validate construct declarations. - Runtime
construction: check provided field initializers and their values against
the declared fields/types. - Assignment: enforce field mutability and
type rules. - Error reporting: explain the source location and
expected/actual type.

The parser should not be responsible for performing runtime type checks,
and the visitor should not have to rediscover declaration syntax from
token strings.

## 4. Feature-by-feature implementation roadmap

The recommended order below builds on existing code rather than
attempting all syntax at once.

### Feature A --- Reliable assignment (`x = value`, `p.x = value`, `p.x += value`)

**Files:** `token.h`, `lexer.c`, `ast.h`, `ast.c`, `parser.c`,
`visitor.h`, `visitor.c`, potentially `value.c`.

**Steps:** 1. Decide whether `=` is allowed as a statement-level
assignment only or as an expression. Prefer statement-level first to
avoid precedence/side-effect surprises. 2. Add AST fields/type for
assignment: target AST, assignment operator, RHS AST. 3. Add parser
functions for assignment target and assignment statement. Parse the
target first, then `=`, `+=`, `-=`, etc., and then the RHS. 4. Add a
`visitorResolveLValue()` helper in `visitor.c` (or a small runtime
module) to resolve a variable/field to its actual stored location. 5.
Add `visitorVisitAssignment()` and dispatch it from `visitorVisit()`. 6.
Add AST cleanup for target and RHS in `astFree()`. 7. Test plain
variable assignment first, then field assignment, then compound
assignment. 8. Ensure every path frees temporary values without freeing
borrowed values.

**Control flow:** parser builds `AST_ASSIGNMENT` → `visitorVisit()`
dispatches → resolve target storage → evaluate RHS → apply operator →
update storage.

### Feature B --- Construct declarations and construct instances

**Files:** `token.h`, `lexer.c`, `ast.h`, `ast.c`, `parser.c`,
`scope.h`, `scope.c`, `value.h`, `value.c`, `visitor.h`, `visitor.c`.

**Suggested AST/runtime separation:** - `AST_CONSTRUCT_DEFINITION`:
construct name, field declarations, method declarations, annotations,
scope. - `AST_CONSTRUCT_INITIALIZER`: construct name plus named field
initializers. - `VALUE_STRUCT`: runtime instance fields. -
`constructDefinition_T` (or AST-backed definition initially): metadata
for field names/types and methods.

**Steps:** 1. Add `AST_CONSTRUCT_DEFINITION` and its fields to `ast.h`;
initialize and free them in `ast.c`. 2. In `parserParseID()` (or a
statement dispatcher), route `construct` to
`parserParseConstructDefinition()`. 3. Implement
`parserParseConstructDefinition()` to parse the construct name, `{`,
field declarations and methods, and `}`. Keep field and method lists
separate. 4. Parse `Player{.x = 1, .y = 1}` as an expression in
`parserParseFactor()` via a new `parserParseConstructInitializer()`. 5.
Parse named initializers into an array of `{fieldName, expression}`
records. Do not represent `.x = 1` as an ordinary variable assignment;
it is initializer syntax. 6. Add a construct-definition registry to the
relevant environment (`scope_T` initially, or a dedicated module/runtime
registry later). 7. Implement `visitorVisitConstructDefinition()` to
register the blueprint without running its methods. 8. Implement
`visitorVisitConstructInitializer()` to look up the blueprint, validate
supplied field names and types, evaluate values, and populate a new
`VALUE_STRUCT`. 9. Add a runtime construct name/type tag to struct
instances if you need to distinguish `Player` from another construct
with identical fields. A plain field-only `VALUE_STRUCT` cannot reliably
tell those types apart. 10. Test empty constructs, full initialization,
missing/unknown/duplicate fields, nested constructs, and deep copying.

**Control flow:** `construct Player { ... }` → AST blueprint → visitor
registers blueprint. `Player{...}` → initializer AST → visitor finds
blueprint → evaluates/validates fields → returns instance value.

**Design question:** decide whether omitted fields receive defaults
(null/zero/default expression) or are errors. Requiring all fields at
first is simpler and safer.

### Feature C --- Typed fields and typed variables (`var x -> float`)

**Files:** `lexer.c`, `token.h`, `ast.h`, `ast.c`, `parser.c`,
`value.h`, `value.c`, `visitor.c`.

**Steps:** 1. Tokenize `->` as one token (`TOKEN_ARROW`), checking it
before treating `-` as a single-character token. 2. Define the initial
type grammar: for example `float`, `string`, `bool`, `Player`; decide
whether types are keywords or ordinary identifiers. 3. Add a type-name
field to variable declaration AST nodes and construct-field
declarations. 4. Update `parserParseVariableDefinition()` to accept
`var x -> float = 1` and possibly declarations without initializers in
construct bodies. 5. Add a type resolver/checker, initially a helper
such as `valueMatchesType(value, typeName, runtimeTypeName)`. 6.
Validate initialized variables and construct fields when values are
created. 7. Enforce the same type rule on later assignments in
`visitorVisitAssignment()`. 8. Decide whether `float` means a general
floating-point number or whether integer and float are distinct types.
Currently all numbers are `VALUE_NUMBER` backed by `double`, so the
language does not distinguish them.

**Control flow:** lexer emits `TOKEN_ARROW` → parser stores declared
type → runtime/type checker validates initializer → assignment path
validates every future write.

### Feature D --- Construct methods and qualified calls (`Player.update(p)`)

**Files:** `lexer.c`, `token.h`, `ast.h`, `ast.c`, `parser.c`,
`scope.h`, `scope.c`, `visitor.c`.

**Steps:** 1. Decide the grammar for dotted names. A robust
representation is a chain of name/member nodes rather than one flat
string. 2. Add AST support for member access/call, such as
`AST_MEMBER_ACCESS` and `AST_MEMBER_CALL`, or represent a call target as
an expression. 3. Update `parserParseFactor()`/identifier parsing to
recognize `Player.update(p)` and `p.x`. 4. Store methods under their
construct definition instead of registering them as unrelated global
functions. 5. At runtime, resolve `Player.update` as a method belonging
to the `Player` blueprint, then execute it with the explicit instance
argument. 6. Keep the language's explicit-parameter style:
`func update(p)` and `Player.update(p)` is coherent and avoids implicit
`this`/`self`. 7. Ensure `Player.printDebug(p)` is rejected if
`printDebug` is marked private and the call originates outside the
allowed construct context.

**Control flow:** parse `Player.update(p)` into qualified-call AST →
resolve `Player` definition → find `update` method → validate arguments
→ create call frame → execute body → return result.

**Important:** your desired sample currently comments
`Player.printDebug(p)` as invalid even before `@private` is implemented.
Pick one rule and keep it consistent. A method without `@private` would
normally be callable unless the language defines all methods as private
by default.

### Feature E --- Private methods (`@private`)

**Files:** `lexer.c`, `token.h` (if annotations use dedicated tokens),
`ast.h`, `ast.c`, `parser.c`, `visitor.c`.

**Steps:** 1. Decide whether annotations are tokens (`@` plus
identifier) or lexed as a single annotation token. 2. Parse annotations
before a function and store flags on the function/method AST
(e.g. `isPrivate`). 3. Validate annotation placement and reject unknown
annotations. 4. During qualified method lookup/call, compare the
method's privacy against the current execution context/owning construct.
5. Produce a clear access error rather than reporting "undefined
function." 6. Add tests for allowed internal calls, disallowed external
calls, and unknown annotations.

**Control flow:** lexer sees `@private` → parser attaches annotation
metadata to method AST → call resolver checks metadata and current
context → allow or reject.

### Feature F --- `while` loops and logical operators

**Files:** `lexer.c`, `token.h`, `ast.h`, `ast.c`, `parser.c`,
`visitor.h`, `visitor.c`.

**Steps for `while`:** 1. Add an `AST_WHILE` node containing condition
and body. 2. Parse `while (condition) { statements }` in statement
parsing. 3. Add `visitorVisitWhile()` and dispatch it in
`visitorVisit()`. 4. Each iteration evaluates the condition; require a
boolean (or define truthiness); stop when false; execute the body when
true. 5. Handle `return` propagation: if a function returns inside a
loop, the loop must stop and pass the return result outward. 6. Decide
what happens when the loop body has no semicolon before `}` and keep
block parsing consistent.

**Steps for `&&` and `||`:** 1. Tokenize `&&` and `||` as dedicated
tokens. 2. Add AST operator support. 3. Add parser precedence layers:
arithmetic → comparisons → `!` → `&&` → `||` (or another documented
choice). 4. Implement boolean type checking in the visitor. 5. Implement
**short-circuit evaluation**: for `a && b`, do not evaluate `b` when `a`
is false; for `a || b`, do not evaluate `b` when `a` is true. This means
these operators cannot be implemented by always evaluating both children
first as the current binary visitor does.

### Feature G --- Modules and imports

Target syntax:

``` dara
define raylib as rl;
import functions;
from functions import hello;
```

**Files:** `lexer.c`, `token.h`, `ast.h`, `ast.c`, `parser.c`,
`scope.h`, `scope.c`, `visitor.c`, and the program entry point.

**Steps:** 1. Specify exact semantics for `define`, `import`, and
`from ... import ...`. For example, distinguish a native library alias
(`raylib as rl`) from a Dara source module (`functions`). 2. Parse each
declaration into its own AST node with module path, alias, and selected
symbols. 3. Decide module search paths, file extensions,
duplicate-import behavior, and whether imports are cached. 4. Create a
module loader/registry that owns module state and exported symbols. 5.
For Dara modules, read source through `fileGetContents()`, then run
lexer → parser → evaluator in a module scope. 6. For native modules,
load a shared library using a platform abstraction and map exported C
functions/constants into Dara values/callables. 7. Implement
`functions:hello()` and `rl:DrawRectangle(...)` using a
namespace-qualified call AST, not by pretending `:` is a normal part of
a function name. 8. Track exports and privacy so modules can control
which names are visible.

**Control flow for a Dara module:** import AST → module loader resolves
path → `fileGetContents()` → `lexerInit()` →
`parserInit()`/`parserParse()` → evaluate module in module scope →
expose requested exports.

**Control flow for a native library:** define/import AST → native loader
opens library → binding layer registers function wrappers/constants →
qualified call resolves binding → converts Dara values to C arguments →
calls C function → converts result back to a Dara value.

**Recommendation:** start with imports between Dara files before dynamic
C loading. Native bindings introduce ABI, type conversion, error
handling, ownership, and platform differences. For raylib, begin with a
small hand-written wrapper module rather than attempting to expose
arbitrary C functions automatically.

### Feature H --- `invoke main` and program exit codes

Target syntax:

``` dara
invoke main {
    var game -> Game = Game.init(...);
    Game.run(game);
    return SUCCESS;
};
```

**Files:** `lexer.c`, `token.h`, `ast.h`, `ast.c`, `parser.c`,
`visitor.c`, program entry point.

**Steps:** 1. Define whether `invoke main` is a special entry-point
block or an ordinary function named `main`. 2. Parse it as `AST_INVOKE`
or as a normal function definition with entry-point metadata. 3. At
startup, register declarations and modules, then execute the selected
entry point. 4. Define `SUCCESS` (and failure codes) as built-in
constants or enum-like values. 5. Define whether `return SUCCESS` exits
only the entry block or also determines the process exit status. 6.
Update the C entry point to convert the Dara result to an integer
process exit code and free runtime resources.

**Control flow:** C `main()` → initialize runtime → load/parse source →
register definitions/imports → locate Dara entry point → execute it →
convert return value to process exit status → clean up.

### Feature I --- Raylib/native functions and `std.dara`

**Files:** eventually a new module/binding subsystem, plus `value.h/.c`,
`visitor.c`, module loading code, and build configuration.

**Suggested stages:** 1. Keep a small set of built-ins in C (`print`,
basic conversion/error functions). 2. Implement Dara modules and
imports. 3. Define a native-function value or callable registry so a
function binding can store a C function pointer plus signature metadata.
4. Write a tiny raylib binding layer for the exact functions you need
(`InitWindow`, `WindowShouldClose`, `BeginDrawing`, `ClearBackground`,
`DrawRectangle`, `EndDrawing`, `CloseWindow`) and constants (`RED`,
`BLUE`). 5. Decide conversion rules for strings, numbers, booleans,
null, structs, and pointers/handles. 6. Add platform-specific
loading/building behind one API so Windows and Linux differences stay
out of the parser and visitor. 7. Put reusable Dara-level helpers in
`std.dara`; keep low-level ABI details in native bindings.

Do not let the parser know what raylib is. It should only parse a
qualified call. The module/binding layer resolves `rl:DrawRectangle` at
runtime.

### Feature J --- Better built-ins and standard library

**Files:** `visitor.c` initially; eventually `stdlib/` or `modules/`
files and module registration code.

-   Finish or remove the empty `builtinFunctionSub`,
    `builtinFunctionMultiply`, and `builtinFunctionDivide`.
-   Fix `builtinFunctionAdd` to use the intended numeric type and return
    a `value_T *` result.
-   Consider replacing the chain of name checks (`if name == "print"`)
    with a built-in registry that maps a name to a callable plus
    metadata.
-   Keep language-level conveniences in `std.dara` when they can be
    written in Dara itself.
-   Define consistent argument-count/type errors and ownership rules for
    built-ins.

## 5. A practical implementation order

A sensible incremental order for your current interpreter is:

1.  **Document and test the current behavior**: values, scopes, return,
    function calls, and ownership.
2.  **General assignment**: `x = value`, then compound assignment.
3.  **Member access**: read `p.x` from a struct value.
4.  **Member assignment**: mutate the stored `p.x`, not a temporary
    copy.
5.  **Construct definitions**: register blueprints with declared fields.
6.  **Named construct initialization**: `Player{.x = 1, .y = 1}`.
7.  **Method storage and qualified calls**: `Player.update(p)`.
8.  **Per-call local scopes**: do this before recursion and serious game
    scripts.
9.  **Typed fields/variables** and assignment-time checks.
10. **`while` loops**, then `&&`/`||` with short-circuiting.
11. **Dara module imports** and exported symbols.
12. **`@private`** once method resolution and execution context exist.
13. **`invoke main`** and process exit status.
14. **Native library bindings**, starting with a small raylib wrapper.
15. **`std.dara`** helpers and a broader standard library.

This order intentionally delays modules and native calls. Constructs and
assignments are the central data model for your desired game scripting
style, so they are worth making solid first.

## 6. A compact feature-to-file index

  -------------------------------------------------------------------------------------------------------------------------
  Feature               Lexer/token          AST                Parser                Runtime/value/scope   Main
                                                                                                            integration
  --------------------- -------------------- ------------------ --------------------- --------------------- ---------------
  Assignment and `+=`   Add tokens if needed Assignment node    Assignment target/RHS Resolve mutable       No
                                                                                      storage; update value 

  `p.x` field access    `.` token            Member-access node Factor/member chain   Read field from       No
                                                                                      struct                

  Construct blueprint   `construct`          Construct          Construct             Construct registry    No
                        identifier (or       definition node    body/fields/methods                         
                        keyword token)                                                                      

  `Player{.x=...}`      `.`, `=`             Initializer node   Named initializer     Build `VALUE_STRUCT`, No
                                                                list                  validate              

  Types `-> float`      `->` token           Type metadata      Typed declarations    Type checking on      No
                                                                                      init/write            

  `Player.update(p)`    `.`                  Qualified/member   Call-target parsing   Resolve method and    No
                                             call                                     frame                 

  `@private`            `@`                  Annotation flags   Annotation parser     Access check          No

  `while`               `while`              While node         Condition/body        Loop evaluator        No
                        identifier/token                                                                    

  `&&`, `||`            Operator tokens      Binary/logical     Precedence layers     Boolean short-circuit No
                                             nodes                                                          

  `define ... as ...`   `as`                 Import/native      Module declaration    Module/native         Yes
                        identifier/token     declaration                              registry              

  `import` / `from`     identifiers/tokens   Import nodes       Import grammar        Module loader/exports Yes

  `invoke main`         identifiers/tokens   Entry node or      Entry grammar         Execute selected      Yes
                                             function metadata                        entry                 

  Raylib C calls        `:` qualified syntax Qualified call     Qualified name        Native callable       Build/runtime
                                                                                      registry              loader

  `std.dara`            import syntax        Module/import      Module parser         Standard module       Module paths
                                             nodes                                    environment           
  -------------------------------------------------------------------------------------------------------------------------

## 7. Current implementation caveats worth tracking

These are not all blockers for the next feature, but they matter as the
language grows:

1.  **The lexer currently has a duplicate source snippet in the supplied
    message.** Ensure the actual project only contains one definition of
    each lexer function.
2.  **`lexerAdvance()` and the main lexer loop call `strlen()`
    repeatedly.** Cache source length in the lexer if performance
    becomes relevant.
3.  **Strings do not handle escape sequences or unterminated-string
    errors** in the shown implementation.
4.  **Identifiers do not support underscores** in the shown
    implementation.
5.  **Numbers are currently digits only**, despite conversion to
    `double`.
6.  **The parser's `parserParseStatements()` assumes it can parse a
    first statement.** Empty blocks and EOF/right-brace handling deserve
    explicit tests; construct bodies and loops will make this more
    important.
7.  **`scopeAddFunctionDefinition()` stores AST pointers.** Ensure the
    AST outlives all function calls, and define one clear owner
    responsible for freeing function ASTs.
8.  **Function parameters use the function body's persistent scope.**
    Move to per-call frames before recursion or more advanced calls.
9.  **The visitor's `shouldReturn` is global to the visitor.** A
    return-state value stored in each call frame is safer for nested
    calls and recursion.
10. **`visitorVisitCompound()` overwrites its previous result without
    freeing it.** If statements return owned values, free or otherwise
    manage each intermediate result before replacing it.
11. **`valueStructGetField()` returns a borrowed pointer.** Make
    ownership explicit in helper names/comments and do not free borrowed
    fields.
12. **Struct copies are deep copies.** Decide whether constructs should
    have value semantics or reference/handle semantics. Your current
    `valueCopy()` implies value semantics, which is workable but should
    be intentional.
13. **The parser exits immediately on errors.** As the language grows,
    consider structured diagnostics with line/column positions and
    cleanup rather than calling `exit()` deep inside parser functions.
14. **`builtinFunctionAdd()` and the arithmetic built-ins are
    incomplete** in the supplied snippet.
15. **Comparison evaluation currently assumes numeric operands.** Decide
    how strings, booleans, null, and construct values should compare.

## 8. Suggested per-feature implementation checklist

For each feature, work through the same sequence:

1.  **Syntax:** write 3--5 examples, including invalid cases.
2.  **Tokens:** decide which new characters or tokens are required.
3.  **AST:** decide exactly what information must survive parsing.
4.  **Parser:** add the smallest parser function that builds that AST.
5.  **Runtime model:** decide what data or environment state is needed.
6.  **Evaluator:** add one visitor function and one dispatch case.
7.  **Ownership:** document which layer owns each allocation and whether
    returned pointers are borrowed or owned.
8.  **Errors:** test missing names, wrong types, wrong argument counts,
    invalid fields, and invalid access.
9.  **Tests:** add one success and one failure case for each syntax
    feature.
10. **Memory:** run Valgrind after each step, as you are already doing.

The most important design principle for Dara is to keep each layer
responsible for one job: **lexer recognizes syntax, parser creates
structure, AST stores meaning, scope resolves names, value represents
data, visitor executes behavior, and the runtime/module layer connects
Dara to native code.** If you keep those boundaries clear, adding
features becomes a repeatable process rather than a hunt through the
codebase.
