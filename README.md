# result

A small error-handling monad for C++26, built on top of `std::expected` and
modelled after Rust's [`anyhow`](https://github.com/dtolnay/anyhow).

```cpp
using result::Result;

Result<int> parse(std::string_view str) {
    if (str.empty())
        return result::fail("empty input");
    return std::stoi(std::string(str));
}

Result<int> doubled(std::string_view s) {
    // Unwrap or return the error with extra context attached
    const int num = TRY(parse(s), "while parsing count");
    return num * 2;
}

Result<std::string> excited_doubled(std::string_view s) {
    const int num = TRY(doubled(s), "while doubling input");
    return  std::format("{}!", num);
}

extern "C++" int main() {
    if (auto result = excited_doubled("23"); result)
        std::println("Got: {}", *result);


    if (auto result = excited_doubled(""); !result)
        std::println(std::cerr, "{}", result.error().report());
        // alternatively: std::cerr << result.error().report() << "\n";
}
```

```
Got: 46!
empty input
    at parse(...)            [main.cppm:14]
    at doubled(...)          [main.cppm:20]
      └── while parsing count
    at excited_doubled(...)  [main.cppm:25]
      └── while doubling input
```

## Building

The library is written using C++ modules. It can be used standalone:

```cmake
FetchContent_Declare(slotmap
        GIT_REPOSITORY https://github.com/inconspicuoususername/slotmap
        GIT_TAG v1
        GIT_SHALLOW TRUE
        SYSTEM)
FetchContent_MakeAvailable(slotmap)
target_link_libraries(target PRIVATE inco::result)
```
...or as a CMake subdirectory:

```cmake
# git clone manually
add_subdirectory(result)
target_link_libraries(target PRIVATE inco::result)
```

#### Requirements

- GCC 16.2 or Clang 22.1. Earlier versions could potentially work. **MSVC will not work** due to usage of statement expressions.
- CMake 4.3+ with C++ module support.

## Usage

`Result<T>` is an extension of `std::expected<T, Error>`. The `Error`
type carries a stack of context frames, each tagged with a message and a
`std::source_location`, so a failure can accumulate context as it propagates and
print a readable trace at the top.

### Creating errors

`result::fail(msg)` returns a `std::unexpected<Error>` tagged with the current
source location. Assigning it into any `Result<T>` produces the error state.

### Adding context

- `TRY(expr)` unwraps a `Result`, or returns `std::unexpected` from the enclosing
  function on failure, tagging the frame with the callsite's location.
- `TRY(expr, ctx)` does the same and also attaches `ctx` as a message on the frame.
- `TRY_IGNORE(expr)` is for `void` functions: it unwraps on success and just
  `return`s on failure (no error is propagated).
- `.with_ctx()`, `.with_ctx(msg)`, and `.with_ctx(callable)` attach context to a
  `Result` inline without early-returning. The callable overload is only invoked
  on the error path.

The `TRY`/`TRY_IGNORE` macros use a GNU statement-expression extension.

### Reporting

`Error::report()` renders the frame stack with the top-level message first,
followed by each frame's function, file, and line. Function signatures are
shortened for readability: template and parameter lists are collapsed to `...`
and GCC module-ownership tags (`Foo@some.module`) are stripped.
