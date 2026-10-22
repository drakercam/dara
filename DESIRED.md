# Dara — Desired Features & Development Roadmap

This document describes planned features for Dara, an interpreted programming language implemented in C. It serves as a development roadmap for language syntax, runtime behavior, memory management, native integrations, and the standard library.

Features described here are **planned unless explicitly marked as implemented**. Proposed syntax is illustrative and may change during implementation.

## Contents

1. [Development Priorities](#1-development-priorities)
2. [Core Language Features](#2-core-language-features)
3. [Chunk Storage System](#3-chunk-storage-system)
4. [Arrays and Other Storage Features](#4-arrays-and-other-storage-features)
5. [Memory Management and Runtime Safety](#5-memory-management-and-runtime-safety)
6. [Standard Library](#6-standard-library)
7. [Modules and Native Integrations](#7-modules-and-native-integrations)
8. [Graphics and Game Development](#8-graphics-and-game-development)
9. [Hot Reloading](#9-hot-reloading)
10. [Implementation Dependencies](#10-implementation-dependencies)
11. [Suggested Development Order](#11-suggested-development-order)

---

## 1. Development Priorities

The roadmap is organized into categories rather than a single flat list. The order of implementation should account for dependencies: a feature that other features rely on should generally be implemented first.

Difficulty estimates are relative to Dara's current C implementation and may change as the runtime evolves.

| Feature | Difficulty | Priority | Notes |
|---|---|---|---|
| Construct definitions and instances | Medium | High | Fields, methods, instance creation, and metadata |
| Designated initialization | Medium | High | Initialize constructs using `.field = value` |
| Chained member access | Medium | High | Access nested fields and construct members |
| Controlled mutation | Medium–High | High | Validate assignments and preserve ownership |
| Logical operators and control flow | Low–Medium | High | Operator precedence, short-circuiting, and execution |
| Reference parameters | Implemented; ongoing refinement | High | Borrowed aliases and mutation of caller-owned values |
| Runtime error propagation and cleanup | Medium–High | High | Release owned resources after runtime failures |
| Chunk storage system | Medium–High | High | Typed and heterogeneous collections, explicit capacity management |
| Fixed-length arrays | Medium | Medium | Predictable indexed storage |
| `@private` annotations | Medium | Medium | Function visibility and access checks |
| Standard-library foundation | Medium–High | High | Built on stable runtime and collection primitives |
| Import and module system | Medium–High | Medium | Loading, symbol resolution, caching, and visibility |
| Native C bindings | Medium–High | Medium | Argument conversion, native types, errors, and return values |
| Raylib module aliasing | Medium–High | Medium | Support aliases such as `raylib as rl` |
| Collection algorithms | Medium | Medium | Sorting, searching, range operations, and filtering |
| Hot reloading | High | Later | Module lifecycle and preservation of runtime state |

## 2. Core Language Features

### 2.1. Construct definitions

**Status:** Partially implemented; continued refinement planned.

Constructs define structured values containing fields and methods.

```dara
construct Player {
    int health;
    int x;
    int y;
};
```

Planned capabilities:

- Define constructs with typed fields.
- Create construct instances using designated initialization.
- Access and modify fields.
- Support nested constructs and chained member access.
- Define methods associated with constructs.
- Validate field assignments against the declared type.
- Preserve ownership rules when fields are copied or replaced.
- Provide clear diagnostics for invalid fields, assignments, and method calls.

Constructs are the foundation for structured game entities, configuration objects, inventory items, and other user-defined data.

### 2.2. Designated initialization

**Status:** Partially implemented.

Construct instances should support initializing fields by name.

```dara
Player player = construct Player(
    .health = 100,
    .x = 10,
    .y = 20
);
```

Designated initialization should integrate with construct metadata and runtime type validation.

Potential future considerations include missing-field behavior, duplicate initializers, default field values, and invalid field names.

### 2.3. Chained member access

**Status:** Partially implemented.

Nested structures should be accessible through chained member expressions.

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

player.stats.score = 100;
```

Member access must cooperate with assignment evaluation, references, and ownership tracking.

In particular, accessing a nested field should not accidentally create an independently owned value when the expression is intended to refer to a field inside an existing construct.

### 2.4. Controlled mutation

**Status:** Planned refinement.

Dara should validate mutations to variables, construct fields, and collection elements.

Goals:

- Reject assignments to invalid targets.
- Validate values against declared types where applicable.
- Preserve the address of a destination when assigning through a reference.
- Avoid freeing borrowed values.
- Prevent double frees and dangling pointers caused by incorrect ownership transfers.
- Eventually enforce private fields and other access restrictions.

Controlled mutation is important for constructs, reference parameters, and the future chunk storage system.

### 2.5. Logical operators and control flow

**Status:** Partially implemented; expansion planned.

Existing control-flow constructs include `if`, `while`, and `for`. The language should continue to develop consistent operator precedence and evaluation rules.

Planned work includes:

- Logical conjunction and disjunction, such as `&&` and `||`, if adopted as the language syntax.
- Short-circuit evaluation.
- Consistent boolean evaluation and comparison semantics.
- Tests for nested conditions and loop execution.
- Clear runtime diagnostics for invalid operations.

The exact operator syntax should be finalized before implementation.

### 2.6. Reference parameters

**Status:** Implemented; continued ownership and lifetime refinement planned.

Reference parameters allow functions to modify existing variables and assignable fields.

```dara
func setHealth(int& health) -> noval {
    health = 100;
};
```

References should remain borrowed aliases, not independently owned values.

Future collection features must preserve these semantics. For example, `chunk.at(index)` should support mutation of an existing element without accidentally freeing or replacing the storage location itself.

---

## 3. Chunk Storage System

**Status:** Planned.

### 3.1. Overview

A `chunk` is intended to be Dara's general-purpose, dynamically sized storage container.

Rather than introducing a large collection hierarchy, Dara should provide one fundamental container with two primary type policies:

- **Heterogeneous chunks:** accept multiple supported runtime value types.
- **Typed chunks:** accept only values matching a declared element type.

Chunks should support indexed access, insertion, explicit capacity growth, and range-based data manipulation.

Potential applications include:

- Grouping game entities and their data.
- Storing inventories and collections of items.
- Managing collections of constructs.
- Sorting, searching, and transforming data.
- Extracting and independently modifying subsets of a larger collection.

The initial implementation should reuse Dara's existing `value_T` representation and ownership conventions.

### 3.2. Construction and type policies

Proposed syntax:

```dara
// Heterogeneous chunk, initial capacity of 1.
chunk data = chunk() -> auto;

// Typed chunk, initial capacity of 16.
chunk numbers = chunk(16) -> int;

// Typed chunk containing Weapon constructs.
chunk weapons = chunk(16) -> Weapon;
```

The intended behavior is:

| Construction | Initial capacity | Element policy |
|---|---:|---|
| `chunk() -> auto` | 1 | Any supported storable value type |
| `chunk(n) -> auto` | `n` | Any supported storable value type |
| `chunk(n) -> int` | `n` | Integers only |
| `chunk(n) -> string` | `n` | Strings only |
| `chunk(n) -> Weapon` | `n` | `Weapon` constructs only |

The type-policy syntax is a proposal and needs to be integrated into Dara's parser and runtime.

A chunk declared without an initializer should initially have the value `noval`, consistent with the desired behavior of uninitialized variables.

```dara
chunk data;

data = chunk() -> auto;
```

An uninitialized chunk and a valid, empty chunk must remain distinct states.

The `auto` policy here means heterogeneous element storage. It should not automatically imply that every `auto` variable in Dara becomes dynamically typed.

The language should also define which values are storable. Initially, chunks should store ordinary supported values rather than `noval` entries or borrowed references with potentially unsafe lifetimes.

### 3.3. Size and capacity

Chunks must distinguish between the number of stored elements and the amount of allocated storage.

- `size()` returns the number of elements currently stored.
- `capacity()` returns the number of elements the allocated storage can hold.

For example:

```dara
chunk numbers = chunk(16) -> int;

print(numbers.size());      // 0
print(numbers.capacity());  // 16
```

The chunk has room for 16 integers but initially contains none.

### 3.4. Explicit growth

**Design decision:** growth should be explicitly controlled by the programmer rather than occurring automatically during insertion.

```dara
chunk numbers = chunk(2) -> int;

numbers.add(10);
numbers.add(20);

// Capacity is exhausted.
numbers.grow(2);

numbers.add(30);
```

The proposed `grow(factor)` operation multiplies the existing capacity by the supplied positive integer.

For example:

- Capacity 2 becomes 4 after `grow(2)`.
- Capacity 16 becomes 32 after `grow(2)`.
- Existing elements remain unchanged.
- Growth does not insert additional elements.

An insertion that exceeds capacity should produce a runtime diagnostic rather than silently allocating more storage.

The implementation must validate growth factors, detect integer overflow, handle allocation failure, and define behavior for invalid capacities.

Growth may relocate the underlying array. References to elements must therefore have documented invalidation rules.

### 3.5. Core operations

The initial chunk API should remain deliberately small.

| Operation | Intended behavior |
|---|---|
| `add(value)` | Append an element if capacity permits |
| `at(index)` | Obtain access to an existing element |
| `size()` | Return the number of stored elements |
| `capacity()` | Return allocated element capacity |
| `grow(factor)` | Explicitly increase capacity |
| `carve(start, end)` | Create an independent chunk containing a range |
| `mend(source, dstStart, dstEnd, srcStart, srcEnd)` | Replace a destination range using another chunk's range |

The names and exact return types may be adjusted to match Dara's existing method-call and assignment semantics.

Only operations necessary for the initial container should be implemented first.

### 3.6. Indexed access and mutation

Chunk elements should support ordinary value access and mutation.

```dara
construct Weapon {
    int damage;
};

chunk weapons = chunk(4) -> Weapon;

weapons.add(construct Weapon(.damage = 50));

weapons.at(0).damage = 100;
```

The mutation must affect the actual stored construct, not a temporary copy.

Implementation requirements:

- Validate indices against the current size.
- Return a mutable access path for valid indexed assignments.
- Preserve construct-field access and assignment semantics.
- Produce a runtime diagnostic for invalid indices.
- Avoid silently retaining pointers into storage that has been invalidated by growth.

The implementation should reuse existing member-access, reference, and assignment machinery where practical.

### 3.7. Carving independent sub-chunks

`carve(start, end)` creates an independent chunk containing copies of the selected elements.

Ranges should use half-open semantics: the starting index is included, and the ending index is excluded.

```dara
chunk data = chunk(16) -> int;

data.add(1);
data.add(2);
data.add(3);
data.add(4);

chunk subData = data.carve(1, 4);
```

The resulting `subData` contains:

```text
[2, 3, 4]
```

Its size is 3, and its initial capacity may be set to 3.

The original chunk remains unchanged. Mutating `subData` must not implicitly modify `data`.

Carving should validate the requested range and produce an independent chunk. It must follow Dara's value-copy and ownership rules; copying a `value_T` structure alone is not necessarily sufficient to create an independently owned copy of its payload.

Carving is intended to support isolated manipulation of a subset of data without requiring the programmer to manually copy each element.

### 3.8. Mending a sub-chunk into another chunk

`mend()` explicitly writes a selected source range back into a destination range.

Proposed syntax:

```dara
data = data.mend(
    subData,
    1, 4,
    0, 3
);
```

The parameters represent:

1. The source chunk.
2. The destination start index.
3. The destination end index.
4. The source start index.
5. The source end index.

With half-open ranges, the example replaces destination indices 1, 2, and 3 with source indices 0, 1, and 2.

If `subData` contains `[20, 30, 40]`, the resulting `data` is:

```text
[1, 20, 30, 4]
```

The original destination range is replaced, and elements outside that range remain unchanged.

Initial `mend()` requirements:

- Source and destination ranges must be valid.
- Source and destination ranges must have equal lengths.
- Element types must be compatible with the destination chunk's type policy.
- The destination must have sufficient capacity for the operation.
- Invalid operations must not leave the destination partially modified.
- Copying or transferring elements must preserve ownership rules.

A future implementation could provide separate operations for inserting or removing a range of different lengths.

Whether `mend()` mutates the destination directly and returns `noval`, or returns the updated chunk, should be settled alongside Dara's existing method and assignment semantics.

### 3.9. Ownership and copying

Chunks own their stored elements.

The runtime must ensure that:

- Each heap allocation has a clear owner.
- Destroying a chunk releases its owned elements and payloads.
- Growing a chunk does not free the payloads of elements being relocated.
- Carving creates valid independent ownership.
- Mending correctly copies or transfers ownership.
- Borrowed references are never freed as owned values.
- Allocation failures do not leave duplicate ownership or partially corrupted containers.

The implementation should reuse the existing `valueFree()`, `valueReplace()`, and value-copying conventions where appropriate.

A chunk should initially use independent allocations for its backing array and any owned payloads required by its elements. A general-purpose arena allocator is not a prerequisite for implementing chunks.

### 3.10. Future collection utilities

Once the core chunk operations are stable, Dara's standard library can build higher-level algorithms on top of them.

Planned candidates:

| Utility | Purpose |
|---|---|
| `sort()` | Sort supported collections or ranges |
| `find()` | Locate a value or element satisfying a search criterion |
| `remove()` | Remove an element while preserving container invariants |
| `clear()` | Remove all elements while retaining capacity |
| Filtering | Select elements matching a condition |
| Range transformations | Operate on selected portions of a chunk |
| Iteration helpers | Visit elements and apply operations to them |

The exact syntax, callback support, comparison behavior, and mutation semantics remain to be designed.

Algorithms should live in the standard library when possible, rather than requiring a dedicated interpreter implementation for every operation.

Typed chunks should allow utilities to validate their element types. Heterogeneous chunks will require explicitly defined comparison or filtering rules.

### 3.11. Chunk implementation strategy

The proposed internal implementation is a C structure backed by a dynamic array of `value_T` entries.

Conceptually:

```c
typedef struct CHUNK_STRUCT {
    value_T* values;

    size_t size;
    size_t capacity;

    /* Element type policy and ownership metadata. */
} chunk_T;
```

This is illustrative rather than a finalized declaration.

Since `value_T` entries have a common representation, one storage implementation can support integers, strings, constructs, and other runtime value types. Their payloads may still require separate allocations.

The chunk should be integrated into the runtime as a built-in value type or equivalent managed runtime object, rather than implemented as an ordinary Dara construct. Its allocation, growth, type checking, and destruction require runtime support.

The final design must specify how chunks are represented in `value_T`, how their values are copied, how assignments work, and how cleanup is integrated with scopes and runtime termination.

---

## 4. Arrays and Other Storage Features

**Status:** Planned.

### 4.1. Fixed-length arrays

Dara should eventually support basic arrays as a complementary storage primitive.

The goal is to provide predictable indexed storage for cases where the collection length is fixed.

Possible syntax:

```dara
int numbers[4];
```

This syntax is illustrative and has not been finalized.

Desired properties:

- Fixed element count.
- Indexed access and mutation.
- Defined element type.
- Bounds checking.
- Predictable storage behavior.
- Integration with Dara's existing runtime ownership model.

Arrays and chunks should not duplicate every implementation detail unnecessarily, but they need not share identical public semantics.

### 4.2. Arrays versus chunks

| Property | Array | Chunk |
|---|---|---|
| Length | Fixed | Changes through insertion and removal |
| Capacity | Fixed | Explicitly growable |
| Type policy | Typically one element type | Typed or heterogeneous |
| Indexed access | Yes | Yes |
| Range extraction | Potential future support | Planned through `carve()` |
| Range replacement | Potential future support | Planned through `mend()` |
| Primary purpose | Predictable indexed storage | Managed, manipulable collections |

The two features should coexist rather than forcing every collection to use the same abstraction.

---

## 5. Memory Management and Runtime Safety

**Status:** Planned refinement.

### 5.1. Centralized runtime error handling

Dara should release interpreter-owned resources when execution fails.

Currently, direct process termination can bypass normal interpreter cleanup. Runtime errors should instead propagate to a centralized execution boundary.

Planned behavior:

1. Detect and report a runtime error.
2. Stop executing the affected program.
3. Unwind interpreter execution safely.
4. Release owned values, scopes, chunks, and runtime resources.
5. Return a failure status to the caller or operating system.

Error reporting and program termination should be separate responsibilities.

Cleanup must not free borrowed references or attempt unsafe recovery from allocation failures.

### 5.2. Ownership consistency

All runtime values must follow a consistent ownership model.

The existing distinction between owned values and borrowed references should be preserved when implementing chunks, arrays, and native bindings.

Future tests should cover:

- Nested constructs.
- Strings and other heap-backed payloads.
- Ordinary value copies.
- Reference parameters and nested field references.
- Whole-value assignment through references.
- Chunk growth and destruction.
- Carving and mending.
- Runtime errors during collection operations.
- Allocation failures.

### 5.3. Automatic cleanup

Chunks and arrays should be released automatically when their owning runtime scope or program lifetime ends, according to their ownership rules.

The programmer should not need to manually free every individual element.

Explicitly controlled chunk growth does not imply manual deallocation. Capacity management and ownership cleanup are separate concerns.

### 5.4. Future arena allocation

A separate arena allocator may be considered later if Dara needs region-based allocation for performance or predictable bulk lifetimes.

It should not be conflated with the public chunk abstraction. Chunks are intended as value containers, while an arena is an allocation strategy.

---

## 6. Standard Library

**Status:** Experimental foundation; substantial work planned.

### 6.1. `std.dara`

The standard library should provide reusable functionality on top of the language's core runtime features.

Planned areas include:

- `std:io` — input and output utilities.
- `std:math` — mathematical operations.
- `std:string` — string manipulation.
- Collection utilities — chunk algorithms and other storage helpers.
- Future modules for files, utilities, and application-specific support.

The exact module layout and import syntax remain subject to the module-system design.

### 6.2. Standard-library design principles

- Prefer library implementations for higher-level utilities when practical.
- Keep built-in runtime support focused on essential operations.
- Reuse constructs, references, chunks, and arrays consistently.
- Provide diagnostics for invalid arguments and unsupported operations.
- Avoid introducing unnecessary language keywords for functionality that can be implemented in the library.

### 6.3. Collection utilities

The chunk system should serve as a foundation for collection algorithms.

For example, a future library might expose sorting and searching utilities without requiring each algorithm to become a built-in runtime operation.

The design depends on stable indexed access, well-defined ownership, type policies, and sufficiently expressive function and callback semantics.

---

## 7. Modules and Native Integrations

**Status:** Planned.

### 7.1. Import system

Dara should support loading declarations from other source files.

Potential syntax:

```dara
import std;
```

Or:

```dara
from std:math import addInt;
```

The exact syntax remains undecided.

Required capabilities:

- Module loading and caching.
- Symbol resolution.
- Qualified access.
- Clear diagnostics for missing modules or symbols.
- Protection against accidental duplicate initialization.
- Integration with namespaces and visibility rules.

The import system should reuse Dara's qualified-name and scope-resolution machinery where practical.

### 7.2. Private declarations

Support `@private` annotations to restrict access to functions and potentially other declarations.

```dara
@private
func internalHelper() -> noval {
    print("internal");
};
```

This syntax is illustrative.

The implementation must define visibility boundaries and enforce access restrictions during symbol resolution or another appropriate validation stage.

Private declarations should integrate with module imports and namespaces rather than existing as an isolated feature.

### 7.3. Native C bindings

Dara should eventually provide a consistent mechanism for exposing native C functions and values to interpreted code.

Required capabilities:

- Register native functions.
- Convert Dara arguments into native representations.
- Convert native results into Dara values.
- Validate argument counts and types.
- Report native-call errors through Dara's diagnostic infrastructure.
- Release temporary allocations and native resources correctly.

Raylib's existing builtin bindings can serve as a starting point, but they are not yet a general-purpose automatic C binding system.

### 7.4. Raylib aliases

The current Raylib integration uses the `rl:` qualified-name prefix.

Future module support should make aliases possible, conceptually:

```dara
import raylib as rl;
```

Aliasing should be handled by the module and symbol-resolution system instead of requiring every library to have a hardcoded prefix.

---

## 8. Graphics and Game Development

**Status:** Incremental expansion planned.

Dara's combination of constructs, references, chunks, and native bindings is intended to support game-development workflows.

Potential applications include:

- Storing entities in typed chunks.
- Grouping heterogeneous gameplay data.
- Managing inventories and item collections.
- Extracting ranges for isolated data processing.
- Sorting and searching entities.
- Calling Raylib or other native graphics APIs.
- Loading reusable modules and hot-reloading game logic.

Chunks should remain general-purpose storage rather than becoming a game-specific entity system. Higher-level entity management can be built on top of them later.

Performance-sensitive features should be evaluated against actual use cases before adding complexity to the core language.

---

## 9. Hot Reloading

**Status:** Planned; high difficulty.

Hot reloading should allow a module or other supported unit of code to be reloaded without restarting the entire program.

Design considerations include:

- Module lifecycle management.
- Preservation or reinitialization of state.
- Re-resolving functions and symbols.
- Handling constructs whose definitions change.
- Ensuring active references do not point to invalid runtime data.
- Cleaning up resources owned by the previous module version.
- Integrating with native libraries and graphics resources where appropriate.

Hot reloading depends on a reliable module system and well-defined ownership and lifetime rules. It should be implemented after those foundations are stable.

---

## 10. Implementation Dependencies

The following relationships should guide implementation order.

| Feature | Depends on | Enables |
|---|---|---|
| Construct definitions | Parser, AST, runtime values | Structured data, typed chunks |
| Designated initialization | Construct metadata, value creation | Convenient construction |
| Chained member access | Construct representation, assignment | Nested mutation, element access |
| Controlled mutation | Type checking, references, ownership | Safer field and element updates |
| Reference parameters | Runtime aliasing and assignment | Mutation helpers and collection algorithms |
| Runtime error cleanup | Ownership tracking, execution control | Safe cleanup for every new feature |
| Chunk runtime type | `value_T`, allocation, copying, destruction | Typed and heterogeneous storage |
| Chunk type policies | Type metadata, construct identity | Restricted collections |
| Explicit growth | Chunk allocation and capacity tracking | Predictable collection allocation |
| `carve()` and `mend()` | Chunk copying, ranges, ownership | Independent range transformations |
| Arrays | Element representation, bounds checking | Fixed-length storage |
| Standard-library algorithms | Stable collection and function semantics | Sorting, searching, filtering |
| Import system | Qualified names, scope resolution | Modular source organization |
| Native binding system | Value conversion, diagnostics, cleanup | Broader C library access |
| Module aliases | Import system, symbol resolution | Flexible native and source modules |
| Hot reloading | Module system, lifetime management | Reloadable application logic |

The core principle is to implement foundational runtime behavior before building convenience features on top of it.

---

## 11. Suggested Development Order

The following sequence balances practical usefulness with implementation dependencies.

### Phase 1 — Stabilize runtime ownership

- [ ] Centralize runtime error propagation.
- [ ] Ensure scopes and owned values are cleaned up on failure.
- [ ] Review value copying and replacement semantics.
- [ ] Add tests for borrowed references and nested constructs.

### Phase 2 — Implement the chunk runtime

- [ ] Define the internal `chunk_T` representation.
- [ ] Decide how chunks are represented in `value_T`.
- [ ] Implement creation and destruction.
- [ ] Implement size and capacity tracking.
- [ ] Implement `add()` with explicit capacity enforcement.
- [ ] Implement indexed access and bounds checking.
- [ ] Implement explicit `grow(factor)`.
- [ ] Implement ownership-aware value copying.
- [ ] Add tests for allocation failure and runtime cleanup.

### Phase 3 — Expose chunks to Dara

- [ ] Add chunk construction syntax.
- [ ] Implement typed and heterogeneous element policies.
- [ ] Validate construct identity for typed chunks.
- [ ] Implement `at()`, `size()`, `capacity()`, and `grow()`.
- [ ] Verify mutable access to stored constructs and nested fields.
- [ ] Define reference invalidation when storage grows.

### Phase 4 — Add range operations

- [ ] Implement half-open range validation.
- [ ] Implement `carve()` using independent storage.
- [ ] Implement `mend()` with equal-length range replacement.
- [ ] Ensure invalid operations leave the destination unchanged.
- [ ] Test nested constructs, strings, type mismatches, and cleanup.

### Phase 5 — Build collection utilities

- [ ] Design function and callback requirements for algorithms.
- [ ] Implement searching.
- [ ] Implement sorting for supported element types.
- [ ] Add removal and clearing operations.
- [ ] Consider filtering and iteration helpers.

### Phase 6 — Expand the language and standard library

- [ ] Implement fixed-length arrays.
- [ ] Expand `std.dara`.
- [ ] Finalize module import syntax.
- [ ] Implement visibility annotations.
- [ ] Generalize native C bindings.
- [ ] Add native module aliases.

### Phase 7 — Advanced runtime features

- [ ] Evaluate arena allocation as a separate capability.
- [ ] Design hot-reload behavior and module lifecycle rules.
- [ ] Expand native graphics support.
- [ ] Profile storage and collection performance before optimizing.

---

## Development Rules

When implementing any feature:

1. Write valid and invalid example programs.
2. Update the AST, parser, visitor, and runtime only where needed.
3. Define ownership and cleanup behavior for every new allocation.
4. Add diagnostic codes for new language and runtime failures.
5. Test interaction with existing constructs, references, and assignments.
6. Run memory diagnostics after ownership changes.
7. Update the language reference, error-code documentation, and examples when behavior changes.

**Guiding principle:** Dara should remain small at the language level while gaining power through a consistent runtime, explicit ownership rules, and a useful standard library. Features should compose with existing constructs and references instead of introducing separate, incompatible mechanisms for similar operations.
