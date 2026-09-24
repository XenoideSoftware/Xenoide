# xenoide-win32xx

Win32++-based sandbox shell. Parallel experiment to `src/xenoidew/` (which uses
winlamb + winlambxe). Demonstrates a 3-zone dock layout — Explorer (left tabs),
Output (bottom tabs), Documents (center tabs) — built with `CDockFrame`,
`CDocker`, and `CDockContainer`.

This subproject is WIP / sandbox per `AGENTS.md`. It does **not** follow the
cross-platform MVP separation; that restructuring is planned for a later pass.

Gated by Conan option `with_win32xx=True`, which sets CMake variable
`XENOIDE_UI_WIN32XX`.
