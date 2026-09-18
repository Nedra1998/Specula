# C++ Style Guide {#dev-cpp-style-guide}

This guide describes how code in this repository is written and organised. It
is split into what **tools enforce** and what **humans enforce in review**. The
bulk of this document covers the second category: the structure and ordering
conventions that `clang-format` and `clang-tidy` can't check.

**Guiding principle:** a reader should be able to open any file and predict
where to find things. These are guidelines, not laws. Break them when there's a
good reason (see [Breaking the Rules](#7-breaking-the-rules)), and leave a
short comment when you do.

[TOC]

## Tool-Enforced Rules

| Concern | Tool | Source of truth |
|---|---|---|
| Whitespace, indentation, line length, brace placement, include sorting | `clang-format` | `.clang-format` |
| Bug-prone patterns, modernisation, readability, naming checks | `clang-tidy` | `.clang-tidy` |
| **Everything else in this document** | Code review | This guide |

If a formatting or lint rule disagrees with this document, the tool
configuration wins and this document should be updated.

Code should be formatted before commit, and CI should fail on `clang-format`
diffs or `clang-tidy` warnings.

## Naming

| Entity | Convention | Example |
|---|---|---|
| Classes, structs | `CamelCase` | `ConnectionPool` |
| Enum types, type aliases, template parameters | `CamelCase` | `LogLevel`, `ValueType` |
| Variables, parameters, data members | `snake_case` | `retry_count` |
| Functions, methods | `snake_case` | `parse_header()` |
| Namespaces | `snake_case` | `network_utils` |
| Files and directories | `snake_case` | `connection_pool.hpp` |
| Constants | `CONSTANT_CASE` | `MAX_RETRIES` |
| Enumerators (enum values) | `CONSTANT_CASE` | `LogLevel::WARNING` |
| Macros | `CONSTANT_CASE` | `PROJECT_ASSERT` |

Additional conventions (proposed, adjust to taste):

- **Private and protected data members** end with a trailing underscore:
  `size_`, `buffer_` when necessary to avoid conflicting with a public member.
  Public data members of plain structs do not.
- **Boolean names** read as predicates: `is_empty()`, `has_value`,
  `should_retry`.
- **File names** match the primary class they contain: `ConnectionPool` lives
  in `connection_pool.hpp` / `connection_pool.cpp`.
- **Extensions:** `.hpp` for headers, `.cpp` for sources.
- **STL-compatibility exception:** types that must satisfy standard library
  requirements keep the standard names (`value_type`, `iterator`,
  `const_iterator`, `size_type`, `begin()`, `end()`). Only do this for types
  meant to interoperate with the STL.


## File Layout

**One primary class per header/source pair.** Small helper types that only
exist to support that class may live alongside it.

### Header files (`.hpp`)

Order of content, top to bottom:

1. **Copyright / license header** (if the project uses one)
2. **Include guard**
3. **Includes**, in groups separated by a blank line (`clang-format` sorts
   *within* each group):
   1. C standard library headers (`<cstdint>`)
   2. C++ standard library headers (`<string>`, `<vector>`)
   3. Third-party library headers
   4. Project headers
4. **Opening namespace**
5. Inside the namespace, in this order:
   1. **Forward declarations**
   2. **Constants** (`inline constexpr`)
   3. **Type aliases** (`using`)
   4. **Enums**
   5. **Structs** (plain data types)
   6. **Classes**
   7. **Non-member functions/operators** that belong to a class, placed
      *immediately after that class*
   8. **Free function declarations**
6. **Closing namespace** with a `// namespace name` comment

**Dependency override:** if something must be declared before another to
compile, dependency order wins over the list above. Use forward declarations to
keep the ideal order where you can.

**Header rules of thumb:**

- Only include what you use, and prefer forward declarations over includes in
  headers.
- Never put `using namespace ...;` in a header.
- Keep function bodies out of headers unless they are templates, `constexpr`,
  or explicit inline.

### Source files (`.cpp`)

1. **Copyright / license header** (if used)
2. **Related header first** (e.g. `connection_pool.hpp` in
   `connection_pool.cpp`), so missing includes in the header are caught
   immediately
3. **Other includes**, grouped as in headers
4. **Opening namespace**
5. **Anonymous namespace** for file-local items, in this order:
   1. Constants
   2. Type aliases, enums, structs
   3. Helper functions (each defined *before* first use, so no forward
      declarations are needed)
6. **Static data member definitions** (if any)
7. **Class member function definitions**, in the **same order as the
   declarations in the header**
8. **Free function definitions**, in the same order as declared in the header
9. **Closing namespace**

**Source rules of thumb:**

- Helpers used by only one function go directly above it (inside the anonymous
  namespace) if they don't fit at the top.
- Prefer an anonymous namespace over `static` for file-local functions.
- Matching header order makes it trivial to jump between declaration and
  definition.

## Class Layout

### Access section order

Use this order, and **do not repeat access specifiers** (one `public:`, one
`protected:`, one `private:`), except when a section genuinely needs splitting:

1. `public:`
2. `protected:`
3. `private:`

Omit sections that are empty. Public API goes first because that is what
callers read.

### Member order within each section

Within each access section, use this order. Skip categories that don't apply.

| # | Category | Notes |
|---|---|---|
| 1 | **Nested types and type aliases** | Aliases first, then nested enums, structs and classes |
| 2 | **Static constants** | `static constexpr` members |
| 3 | **Static factory methods** | `static Foo create(...)`, `static Foo from_string(...)` |
| 4 | **Constructors** | Default, then parameterised, then copy, then move |
| 5 | **Destructor** | Directly after constructors |
| 6 | **Assignment operators** | Copy assignment, then move assignment |
| 7 | **Conversion operators** | `explicit operator bool()`, `operator std::string()` |
| 8 | **Data accessors** | Getters, then setters, in the order the data members appear |
| 9 | **Iterators** | `begin`, `end`, `cbegin`, `cend`, `rbegin`, `rend`, in that order |
| 10 | **Element access operators** | `operator[]`, `operator()`, `operator->`, `operator*`, `at()`, `front()`, `back()` |
| 11 | **Comparison / boolean operators** | `==`, `!=`, `<=>` (or `<`, `<=`, `>`, `>=`), `operator!` |
| 12 | **Arithmetic operators** | Unary (`-`, `+`) first, then **grouped by operation**: each compound assignment sits directly before its binary form (`+=` then `+`, `-=` then `-`, ...), then increment/decrement. See [4.3](#43-arithmetic-operator-grouping) |
| 13 | **Other methods** | The class's main behaviour: `const` methods before non-const, or grouped by function (see below) |
| 14 | **Static methods** | Non-factory static utilities |
| 15 | **Friend declarations** | Last in the final section, just before data members |
| 16 | **Data members** | Almost always in `private:`, always last |

Notes:

- **The rule of five stays together.** Constructors, destructor and assignment
  operators (categories 4-6) should be adjacent, so it's obvious at a glance
  that all five have been considered. If you `= default` or `= delete` some of
  them, still list them explicitly when any one of them is declared.
- **Capacity and state queries** (`size()`, `empty()`, `capacity()`) belong
  with data accessors (8) for value-like classes, or with iterators (9) for
  containers.
- **Category 13 grouping.** For classes with many methods, group them by
  responsibility using a short comment (`// Connection lifecycle`, `//
  Statistics`) rather than by keyword. Within a group, order by importance:
  primary operations first, small helpers last.
- **Private helper methods** go in the `private:` section under category 13,
  after any private static constants and before data members.
- **Data members** go last so the public interface stays at the top. Order them
  by logical grouping, and keep members that are initialised together adjacent.
  (Declaration order is initialisation order, so mind `-Wreorder`.)

### Arithmetic operator grouping

Arithmetic operators are grouped **by operation**, not by kind. The compound
assignment form sits directly before the binary form it pairs with, so
everything about addition is in one place, everything about subtraction in
another, and so on.

Order:

1. Unary operators (`+x`, `-x`)
2. One group per operation, in the order `+`, `-`, `*`, `/`, `%`, then bitwise
   and shift operators if present (`&`, `|`, `^`, `<<`, `>>`). Each group is
   the compound assignment (`op=`) followed by the binary operator (`op`)
3. Increment and decrement: prefix before postfix (`++x`, `x++`, `--x`, `x--`)

```cpp
  // Arithmetic
  Vec3 operator-() const;

  Vec3& operator+=(const Vec3& rhs);
  Vec3 operator+(const Vec3& rhs) const;

  Vec3& operator-=(const Vec3& rhs);
  Vec3 operator-(const Vec3& rhs) const;

  Vec3& operator*=(double scalar);
  Vec3 operator*(double scalar) const;

  Vec3& operator/=(double scalar);
  Vec3 operator/(double scalar) const;
```

Notes:

- **Overloads of the same operation stay in the same group.** For example,
  `operator*=(double)` and `operator*=(const Matrix&)` are both listed before
  the binary `operator*` overloads.
- **Non-member binary operators.** If a binary operator is a free function
  (needed for symmetric conversions or `scalar * vec`), keep the compound
  assignment in the class and place the free operator after the class,
  following the non-member rule in 3.1. Use the same operation order there, and
  separate the group with a blank line:

```cpp
  Vec3 operator+(Vec3 lhs, const Vec3& rhs);
  Vec3 operator-(Vec3 lhs, const Vec3& rhs);
  Vec3 operator*(double scalar, const Vec3& vec);
```

- **Implement binary operators in terms of the compound form** (`a + b` as a
  copy of `a` followed by `+=`), which is why the compound form comes first in
  each group.
- **Comparison operators are not part of this grouping.** They stay together in
  their own group (category 11).


### Inline definitions in classes

- Define inline members (one-line getters, defaulted functions) in-class.
- Anything longer than a few lines goes in the `.cpp` file.
- Do not mix styles in a single accessor group; if one getter needs a `.cpp`
  definition, that's fine, but keep the declaration order.

### Special cases

**Interface / abstract base classes**
Order: destructor (`virtual ~Foo() = default;`), then pure virtual methods,
then non-virtual helpers. Constructors are typically `protected`.

**Derived classes**
Group overrides together in the order the base class declares them, and mark
each with `override` (or `final`).

**Value types (e.g. vectors, money, timestamps)**
Full order applies. Comparison and arithmetic operators are usually the bulk of
the class, so keep them grouped and in the order in the table.

**Container-like types**
Follow the STL conventions: types, constructors, assignment, element access,
iterators, capacity, modifiers, then non-member functions.

## Struct, Enum and Constant Conventions

### Structs vs classes

- Use a **`struct`** for passive data with no invariants: all members public,
  no private state, at most trivial constructors or helper methods.
- Use a **`class`** as soon as there are invariants, private state, or
  non-trivial behaviour.
- Never mix: a type with any private member is a `class`.

**Struct member order:**

1. Nested types and aliases
2. Static constants
3. Constructors (if any)
4. Methods (keep these few and simple)
5. Data members (the data *is* the interface, so it goes first)

### Enums

- Always use **`enum class`**, and specify the underlying type when it matters
  (`enum class LogLevel : std::uint8_t`).
- Enum type: `CamelCase`. Enumerators: `CONSTANT_CASE`.
- Order enumerators logically: by value, severity, or lifecycle order. Only
  alphabetise when there's no natural order.
- If you need a sentinel, name it `COUNT` and put it last. Do not rely on it
  for anything but array sizing.

```cpp
enum class LogLevel : std::uint8_t {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
};
```

### Constants

- Prefer `constexpr` over `const`, and `const`/`constexpr` over `#define`.
- In headers, use **`inline constexpr`** at namespace scope to avoid duplicate
  definitions.
- Class-specific constants are `static constexpr` members, listed near the top
  of the class.
- File-local constants go at the top of the anonymous namespace in the `.cpp`
  file.
- Group related constants together with a comment; order them by relationship,
  not alphabetically.
- Avoid "magic numbers": if a literal needs explaining, it needs a name.

## Breaking the Rules

These are guidelines. Reasonable exceptions include:

- **Dependency requirements:** a type must be defined before another can use
  it.
- **Readability wins:** keeping a tightly coupled getter/setter pair or an
  operator pair (`==`/`!=`) adjacent even if the table would separate them.
- **Generated code, third-party code, and vendored code** follow their own
  conventions.
- **Interop with an external API** that dictates names or ordering (e.g. STL
  requirements, framework callbacks).
- **Historical code:** don't reorder existing files purely for style. Bring
  files into line when you're making substantial changes to them anyway.

When you deviate, add a one-line comment explaining why if it isn't obvious.

## Examples

### Header

```cpp
// connection_pool.hpp
#pragma once

#include <cstdint>

#include <chrono>
#include <string>
#include <vector>

#include <third_party/some_library.hpp>

#include "project/connection.hpp"

namespace network_utils {

class Connection;  // forward declaration

inline constexpr std::size_t DEFAULT_POOL_SIZE = 8;

using Milliseconds = std::chrono::milliseconds;

enum class PoolState : std::uint8_t {
  IDLE,
  ACTIVE,
  DRAINING,
};

struct PoolConfig {
  std::size_t max_size = DEFAULT_POOL_SIZE;
  Milliseconds timeout{5000};
};

class ConnectionPool {
 public:
  // Types
  using ConnectionList = std::vector<Connection>;

  // Constants
  static constexpr std::size_t MAX_CONNECTIONS = 1024;

  // Factories
  static ConnectionPool from_config(const PoolConfig& config);

  // Rule of five
  ConnectionPool();
  explicit ConnectionPool(const PoolConfig& config);
  ConnectionPool(const ConnectionPool& other);
  ConnectionPool(ConnectionPool&& other) noexcept;

  ~ConnectionPool();

  ConnectionPool& operator=(const ConnectionPool& other);
  ConnectionPool& operator=(ConnectionPool&& other) noexcept;

  // Conversion
  explicit operator bool() const;

  // Accessors
  const PoolConfig& config() const { return config_; }
  PoolState state() const { return state_; }
  std::size_t size() const;

  // Iterators
  ConnectionList::iterator begin();
  ConnectionList::iterator end();
  ConnectionList::const_iterator begin() const;
  ConnectionList::const_iterator end() const;

  // Element access
  Connection& operator[](std::size_t index);
  const Connection& operator[](std::size_t index) const;

  // Comparison
  bool operator==(const ConnectionPool& other) const;

  // Connection lifecycle
  Connection& acquire();
  void release(Connection& connection);

  // Shutdown
  void drain();

 private:
  // Helpers
  void grow_by(std::size_t count);

  // Data
  PoolConfig config_;
  PoolState state_ = PoolState::IDLE;
  ConnectionList connections;
};

// Non-member functions belonging to the class
void swap(ConnectionPool& lhs, ConnectionPool& rhs) noexcept;

// Free functions
ConnectionPool make_default_pool();

}  // namespace network_utils
```

### Source

```cpp
// connection_pool.cpp
#include "network_utils/connection_pool.hpp"

#include <algorithm>
#include <stdexcept>

namespace {

constexpr std::size_t GROWTH_FACTOR = 2;

std::size_t next_capacity(std::size_t current) {
  return std::max<std::size_t>(1, current * GROWTH_FACTOR);
}

}  // namespace

namespace network_utils {

// ConnectionPool: same order as the header declaration
ConnectionPool ConnectionPool::from_config(const PoolConfig& config) { /* ... */ }

ConnectionPool::ConnectionPool() = default;
ConnectionPool::ConnectionPool(const PoolConfig& config) : config_(config) {}
// ... remaining rule-of-five members ...

std::size_t ConnectionPool::size() const { return connections_.size(); }

// ... iterators, operators, lifecycle, private helpers ...

void swap(ConnectionPool& lhs, ConnectionPool& rhs) noexcept { /* ... */ }

ConnectionPool make_default_pool() { return ConnectionPool{}; }

}  // namespace network_utils
```

## Quick reference

### Class Member Order

1. Nested types / aliases
2. Static constants
3. Static factory methods
4. Constructors (default, parameterised, copy, move)
5. Destructor
6. Assignment operators (copy, move)
7. Conversion operators
8. Data accessors (getters, setters)
9. Iterators
10. Element access operators
11. Comparison / boolean operators
12. Arithmetic operators (unary, then per operation: += then +, -= then -, ..., then ++/--)
13. Other methods (grouped by responsibility)
14. Static methods
15. Friend declarations
16. Data members
