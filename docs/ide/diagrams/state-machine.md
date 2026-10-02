# Application State Machine

Mermaid `stateDiagram-v2`. Renderable on GitHub, GitLab, Obsidian, and the Mermaid Live Editor.

```mermaid
stateDiagram-v2
    [*] --> Idle

    Idle --> Editing : file.opened

    Editing --> Compiled      : compilation.succeeded
    Editing --> CompileError  : compilation.failed

    CompileError --> Compiled     : compilation.succeeded
    CompileError --> CompileError : compilation.failed

    Compiled --> Editing      : content changed
    CompileError --> Editing  : content changed

    Compiled --> Debugging : debug.started
    Debugging --> Paused   : debug.paused
    Paused --> Debugging   : debug resumed
    Debugging --> Compiled : debug.stopped
    Paused --> Compiled    : debug.stopped

    Idle         --> Idle : cmd.file.new
    Editing      --> Idle : cmd.file.new
    Compiled     --> Idle : cmd.file.new
    CompileError --> Idle : cmd.file.new
    Debugging    --> Idle : cmd.file.new
    Paused       --> Idle : cmd.file.new
```

## Notes

- `cmd.file.new` is valid in all states; it always returns to `Idle`.
- `CompileError → CompileError` represents a re-compile that still fails.
- The `Debugging` / `Paused` sub-cycle maps directly to the RenderDoc capture / step loop.
- `content changed` is an internal editor event, not a bus message; it is detected locally by the GUI process.
