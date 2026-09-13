
# Xenoshader

## Disclaimer

NOTE: This project it a test of the AI-assisted development methodology that I'm refining: The strategy is to give several architectural constraints to the AI in order to quickly generate the major application components and connect them in a way that is easily maintable by humans and/or AI agents in the future. These generated application components should be cross-functional, have low coupled dependencies, be highly reusable, and most importantly, should support one or more *features* (see the specs/ folder). Let's remember that a *feature* is defined as the composition of several *components*, plus some ad-hoc glue code. After several iterations we should give form to that glue code.

## Introduction
Xenoshader is a *SDI* (Single Document Interface) application specializing at *editing*, *compiling* and *debugging* *GLSL shaders*.

NOTE: An SDI app is an application that contains a MenuBar, a single document editing area, and a set of docked panels for diagnostics and output. It manages one file at a time.

## Functional Specs / Archtypes

### SDI

A *SDI* application is one that:
- Allows the user to edit a single source code at a time -> should integrate a single `xenoide-ui-codeeditor`, and supports the *editing* feature.
- Should contain a MenuBar with the typical SDI allowed application commands (File, Edit, View, Help). These menus will also support the *editing* feature, because they allow to create, save, load, etc, GLSL files.
- Should contain a *diagnostics* or *build errors* UI component (`xenoide-ui-build-diagnostics`), that displays notes, diagnostics, linting / build errors, etc, for the current source code being edited.
- Should contain a *terminal* UI component, that allows to open one or more terminal windows in order to commit Git changes, call AI agents, and other tasks.
- Should automatically be listening to background changes in the currently edited file. When an external change is detected, the user must be prompted to choose whether to reload the file from disk or keep the in-memory version.

### GLSL Shaders

- Will complement the *editing* and *compiling* features, giving GLSL-specific knowledge to the editor to display parameters, compilation errors, hover information, and so on. This is supported by embedding the `glslang` library directly (rather than invoking an external process), which removes the runtime dependency on external tools and simplifies distribution. The embedded `glslang` is exposed externally as a **GLSL Language Server** process, implementing the *Language Server Protocol* (LSP), so that it can serve language intelligence to the editor and any other future client.
- GLSL `#include` resolution must be handled by the language server, using a configurable set of include search paths defined per-project.
- A **uniform/attribute inspector** panel (`xenoide-ui-uniform-inspector`) will display the active uniforms, their types, and binding points derived from the compilation output of the current shader.
- **SPIR-V inspection** is in scope. After a successful compilation, the resulting SPIR-V binary can be disassembled (via `spirv-cross` or `spirv-dis`) and displayed in a dedicated `xenoide-ui-disassembly` panel. This panel shows the SPIR-V text alongside the original GLSL source for cross-reference.
- The *debugging* feature will be implemented via the **RenderDoc API**. RenderDoc provides a vendor-neutral capture and replay mechanism that works across Intel, NVIDIA, and AMD hardware, avoiding the fragmentation of vendor-specific OpenGL extensions.

## Software Architecture

### Abstract

This app is structured using a *client-server architecture* split across **multiple processes from the start**. The SDI GUI process has the role of the *client*, and the collection of background services are independent *server* processes. Both sides communicate through a *message bus*, which is fully decoupled from any GUI framework event loop.

The multi-process split is intentional: it provides crash isolation (a GPU driver crash or a runaway compilation does not kill the editor), enables future remote development scenarios, and makes each service independently testable. Every background capability — the GLSL language server, the RenderDoc integration, the filesystem watcher — runs in its own process.

The defined processes are:

| Process | Role |
|---|---|
| `xenoshader` | GUI client (Qt 6 SDI application) |
| `xenoshader-langserver` | GLSL Language Server (LSP, wraps embedded `glslang`) |
| `xenoshader-rdoc` | RenderDoc integration service |
| `xenoshader-fswatcher` | Filesystem watcher background service |

### Message Bus

Both the client-side and backend-side components *subscribe*/*listen* and *emit*/*send* typed *messages*. This message bus is implemented as a dedicated library, `xenoide-msgbus`, which is the **only** component that directly depends on ZeroMQ and protobuf. All other components depend exclusively on the typed C++ service API that `xenoide-msgbus` exposes, making the transport swappable and the rest of the codebase testable without a running ZeroMQ instance.

The layering is:

| Layer | Responsibility | Technology |
|---|---|---|
| Transport | Moving bytes between processes | ZeroMQ (`zeromq`) |
| Serialization | Encoding/decoding messages | Protocol Buffers (`protobuf`) |
| Message Bus | Routing, subscription, request/reply patterns | `xenoide-msgbus` library |
| Service API | Domain-typed C++ interface for the rest of the code | Thin wrappers over the bus |

In order to support this architecture, there will be an *sdi* app framework that contains the initial UI along with predefined standard *messages* (such as *file open*, *file save*, *file conflict detected*, etc.). This framework integrates a base client and server and implements the basic communication protocol for message orchestration.

### Actions, Commands, and Events

The message bus carries two fundamentally different kinds of messages. Mixing them leads to ambiguity about who can send what, and who should receive it.

**Commands** are imperative instructions sent *to* a service. They are consumed by exactly one handler (ZeroMQ PUSH/PULL pattern). They may be emitted by an Action (user-triggered) or programmatically by any component.

**Events** are declarative announcements emitted *by* a service after something has happened. They are broadcast to all interested subscribers (ZeroMQ PUB/SUB pattern). No Action maps to an event directly.

**Actions** are the UI-layer objects that bridge user intent and commands. Each Action:
- Has a visual representation (menu item, toolbar button, keybinding).
- Has an enabled/disabled state driven exclusively by the Application State Machine.
- When activated, emits exactly one Command onto the bus.

This keeps the bus dumb and fast. State enforcement happens at the Action layer — a disabled Action never emits its Command, so the bus router never needs to drop messages based on app state.

#### Actions

| Action | Command Emitted | Valid States |
|---|---|---|
| `FileNewAction` | `cmd.file.new` | `Idle`, `Editing`, `Compiled`, `CompileError` |
| `FileOpenAction` | `cmd.file.open` | `Idle`, `Editing`, `Compiled`, `CompileError` |
| `FileSaveAction` | `cmd.file.save` | `Editing`, `Compiled`, `CompileError` |
| `FileSaveAsAction` | `cmd.file.save_as` | `Editing`, `Compiled`, `CompileError` |
| `ExitAction` | `cmd.app.exit` | *(all)* |
| `UndoAction` | `cmd.edit.undo` | `Editing` |
| `RedoAction` | `cmd.edit.redo` | `Editing` |
| `CutAction` | `cmd.edit.cut` | `Editing` |
| `CopyAction` | `cmd.edit.copy` | `Editing`, `Compiled`, `CompileError` |
| `PasteAction` | `cmd.edit.paste` | `Editing` |
| `GoToDeclarationAction` | `cmd.edit.go_to_declaration` | `Editing`, `Compiled`, `CompileError` |
| `CompileAction` | `cmd.shader.compile` | `Editing`, `CompileError` |
| `DebugAction` | `cmd.debug.start` | `Compiled` |
| `ToggleBreakpointAction` | `cmd.debug.toggle_breakpoint` | `Compiled`, `Debugging`, `Paused` |

Actions are registered in a central `ActionRegistry`. The registry observes state machine transitions and enables/disables the relevant actions in bulk.

#### Commands

Commands emitted by Actions or programmatically by any component:

| Command | Description |
|---|---|
| `cmd.file.new` | Discard current document, reset to blank state |
| `cmd.file.open` | Open a file picker and load a GLSL file |
| `cmd.file.save` | Save the current document to its existing path |
| `cmd.file.save_as` | Save the current document to a new path |
| `cmd.file.reload` | Reload the current document from disk (used programmatically on conflict resolution) |
| `cmd.app.exit` | Initiate application shutdown sequence |
| `cmd.edit.undo` | Undo last edit |
| `cmd.edit.redo` | Redo last undone edit |
| `cmd.edit.cut` | Cut selected text |
| `cmd.edit.copy` | Copy selected text |
| `cmd.edit.paste` | Paste from clipboard |
| `cmd.edit.go_to_declaration` | Navigate to the declaration of the symbol under the cursor |
| `cmd.shader.compile` | Compile the current GLSL source via the language server |
| `cmd.debug.start` | Begin a RenderDoc capture session |
| `cmd.debug.toggle_breakpoint` | Toggle a breakpoint at the current cursor line |

#### Events

Events emitted by services; no Action maps to these directly:

| Event | Emitted by | Description |
|---|---|---|
| `file.opened` | FileService | A file was successfully loaded into the editor |
| `file.saved` | FileService | A file was successfully written to disk |
| `file.conflict_detected` | `xenoshader-fswatcher` | The file on disk changed while the editor holds unsaved modifications |
| `compilation.succeeded` | `xenoshader-langserver` | Compilation produced a valid SPIR-V binary |
| `compilation.failed` | `xenoshader-langserver` | Compilation produced errors; diagnostics are attached |
| `diagnostics.updated` | `xenoshader-langserver` | A new set of diagnostic items is available for the current source |
| `debug.started` | `xenoshader-rdoc` | A RenderDoc capture session is active |
| `debug.paused` | `xenoshader-rdoc` | The debugger paused at a shader invocation |
| `debug.stopped` | `xenoshader-rdoc` | The debug session ended |

### Diagnostic Data Model

All diagnostic items (from `glslang`, the filesystem watcher, or future LSP sources) share a single protobuf message type defined in `message.proto`:

```
DiagnosticItem {
  string   file_path
  uint32   line
  uint32   column
  enum     severity  // NOTE, WARNING, ERROR
  string   message
  string   source    // e.g. "glslang", "fswatcher"
}
```

This avoids duplication and lets the `xenoide-ui-build-diagnostics` panel consume diagnostics from any backend uniformly.

### Application State Machine

The application lifecycle follows a state machine that controls which messages and UI actions are valid at any given time:

| State | Description |
|---|---|
| `Idle` | No file open |
| `Editing` | File open, not compiled |
| `Compiled` | File compiled successfully, SPIR-V available |
| `CompileError` | Compilation failed, diagnostics available |
| `Debugging` | RenderDoc capture session active |
| `Paused` | Debugger paused at a shader invocation |

Menu items and toolbar buttons are enabled/disabled based on the current state via the `ActionRegistry`. State enforcement happens entirely at the Action layer — the bus router is stateless and never drops messages.
