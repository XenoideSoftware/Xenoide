# CPP guidelines

## Standard & Language Features
- **Strict C++17**: Code must strictly conform to C++17. Do not use C++20 features (e.g., concepts, ranges library, `std::span` unless provided by `ms-gsl`/`gsl-lite` and the cpp-backport library, coroutines).
- **Vendor Extensions Disabled**: Do not rely on compiler-specific non-standard extensions.

## Zero-Warning Tolerance
- All code is compiled All Warnings and Warnings as Errors enabled.

## Design Principles & Idioms
- **RAII & Memory Safety**: Never leak raw pointers. Use `std::unique_ptr` for exclusive ownership and `std::shared_ptr` only when ownership is genuinely shared.
- **Value Semantics & Views**: Prefer `std::string_view` for read-only string parameters. Pass complex types by const-reference unless passing by value for sink parameters.
- **Error Handling**:
  - In the engine core and performance-critical loops, avoid throwing exceptions. Prefer monadic types (`tl::expected`, `std::optional`) or explicit result codes.
  - Ensure assertions (`XE_ASSERT` or equivalent) are used to enforce invariants in debug builds.
- **Namespaces**:
  - Engine code belongs in `namespace xe { ... }` or specific sub-namespaces (`xe::graphics`, `xe::math`, etc.).
  - IDE code belongs in `namespace xenoide { ... }`.
- **Include Order**:
  1. Main module header (e.g., `#include "MyClass.h"`).
  2. Subsystem internal headers.
  3. Third-party library headers (e.g., `<fmt/format.h>`, `<tl/expected.hpp>`).
  4. Standard library headers (e.g., `<vector>`, `<string>`, `<memory>`).
