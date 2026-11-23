# Xenoshader

## Disclaimer

NOTE: This project is a test of an AI-assisted development methodology: give the AI several architectural constraints to generate application components that are cross-functional, low-coupled, highly reusable, and that support one or more *features* (a feature being the composition of several *components* plus ad-hoc glue). After several iterations the glue should take a stable form.

## Introduction

Xenoshader is an *SDI* (Single Document Interface) application for *editing*, *compiling*, and *debugging* *GLSL shaders*.

In the domain model (@DOMAIN-MODEL.md), an SDI application is a **Document Manager** restricted to one **Document**, shown in a single **Editor**, surrounded by docked panels. It manages one **Source** at a time.

## Domain Instantiation

- The **Document Manager** holds exactly one **Document**, displayed in one **Editor** (`xenoide-ui-codeeditor`).
- The **Source** is a GLSL file, opened standalone or within a **Project** that supplies `#include` search paths.
- Docked panels (Editors/Views): `xenoide-ui-build-diagnostics` (Diagnostics), `xenoide-ui-terminal` (Terminal), `xenoide-ui-uniform-inspector`, and `xenoide-ui-disassembly` (SPIR-V).

## Tools

Xenoshader drives these Tools (@GLOSSARY.md):

| Tool | Kind | Role |
|---|---|---|
| `xenoshader-langserver` — GLSL Language Server | Server Tool | Embeds `glslang`; LSP; compilation; diagnostics; hover; go-to-declaration; resolves `#include` paths per project. |
| `xenoshader-rdoc` — RenderDoc integration | Server Tool (Debugger) | GPU capture/replay; the backend of the Debug Session. |
| `xenoshader-fswatcher` — Filesystem watcher | Server Tool (File Watcher) | Monitors the open Source; emits `file.conflict_detected`. |

Each Tool advertises **Tool Capabilities**, which gate the available Commands (e.g. the Debugger advertises capture/replay; the Language Server advertises hover and go-to-declaration).

## Software Architecture

### Processes

Client-server, multi-process from the start. The SDI GUI process is the *client*; background capabilities are independent *server* processes. Communication goes through a **Message Bus** decoupled from any GUI event loop.

| Process | Role |
|---|---|
| `xenoshader` | GUI client (Qt 6 SDI application) |
| `xenoshader-langserver` | GLSL Language Server |
| `xenoshader-rdoc` | RenderDoc integration (Debugger) |
| `xenoshader-fswatcher` | Filesystem watcher |

### Message Bus

Typed messages over a bus implemented in `xenoide-msgbus`, the only component that depends directly on ZeroMQ and protobuf. Everything else depends on the typed C++ service API, keeping the transport swappable and the rest testable without a running ZeroMQ instance.

| Layer | Responsibility | Technology |
|---|---|---|
| Transport | Moving bytes between processes | ZeroMQ (`zeromq`) |
| Serialization | Encoding/decoding messages | Protocol Buffers (`protobuf`) |
| Message Bus | Routing, subscription, request/reply | `xenoide-msgbus` |
| Service API | Domain-typed C++ interface | Thin wrappers over the bus |

### Messages

The bus carries two kinds of Message (@ARCHITECTURE.md):

- **Command** — an imperative instruction consumed by exactly one handler (PUSH/PULL).
- **Event** — a declarative announcement broadcast to all subscribers (PUB/SUB).

**Actions** are UI-layer objects bridging user intent and Commands: each has a visual representation, an enabled/disabled state driven by the application state machine, and emits exactly one Command when activated. State enforcement happens at the Action layer, so the bus never drops messages based on state.

#### Actions

| Action | Command | Valid States |
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

Actions are registered in the **ActionRegistry**, which observes state transitions and enables/disables the relevant actions in bulk.

#### Commands

Commands emitted by Actions or programmatically:

| Command | Description |
|---|---|
| `cmd.file.new` | Discard the current Document, reset to a blank (unsaved) Document |
| `cmd.file.open` | Open a file picker and load a GLSL Source into a Document |
| `cmd.file.save` | Save the current Document to its existing Source path |
| `cmd.file.save_as` | Save the current Document to a new path (creating a Source) |
| `cmd.file.reload` | Reload the Document from its Source on disk (conflict resolution) |
| `cmd.app.exit` | Initiate application shutdown |
| `cmd.edit.undo` | Undo last edit |
| `cmd.edit.redo` | Redo last undone edit |
| `cmd.edit.cut` | Cut selected text |
| `cmd.edit.copy` | Copy selected text |
| `cmd.edit.paste` | Paste from clipboard |
| `cmd.edit.go_to_declaration` | Navigate to the declaration of the Symbol under the cursor |
| `cmd.shader.compile` | Compile the current Source (a single-Source Build) via the Language Server |
| `cmd.debug.start` | Begin a Debug Session (RenderDoc capture) |
| `cmd.debug.toggle_breakpoint` | Toggle a Breakpoint at the current cursor line |

#### Events

Events emitted by services; no Action maps to these directly:

| Event | Emitted by | Description |
|---|---|---|
| `file.opened` | FileService | A Source was loaded into the Document |
| `file.saved` | FileService | The Document was written to its Source on disk |
| `file.conflict_detected` | `xenoshader-fswatcher` | The Source on disk changed while the Document holds unsaved modifications |
| `compilation.succeeded` | `xenoshader-langserver` | The Compile produced a valid SPIR-V Artifact |
| `compilation.failed` | `xenoshader-langserver` | The Compile produced errors; Diagnostics attached |
| `diagnostics.updated` | `xenoshader-langserver` | A new set of Diagnostic items is available |
| `debug.started` | `xenoshader-rdoc` | The Debug Session is Running |
| `debug.paused` | `xenoshader-rdoc` | The Debug Session is Paused |
| `debug.stopped` | `xenoshader-rdoc` | The Debug Session ended |

### Diagnostics

All diagnostic items share the domain **Diagnostic** shape (@DOMAIN-MODEL.md): a severity, the producing Tool, and a location in a Source:

```
Diagnostic {
  source_path   // Source location
  line
  column
  severity      // NOTE, WARNING, ERROR
  message
  tool          // e.g. "glslang", "fswatcher"
}
```

This avoids duplication and lets `xenoide-ui-build-diagnostics` consume diagnostics from any Tool uniformly.

### Application State Machine

Xenoshader's concrete state machine is composed from the domain building blocks (`Document.dirty`, `Build.status`, `Debug Session.state`):

| State | Meaning (domain-derived) |
|---|---|
| `Idle` | No Document open |
| `Editing` | Document open, not compiled |
| `Compiled` | Last Compile (single-Source Build) Succeeded |
| `CompileError` | Last Compile Failed |
| `Debugging` | Debug Session Running |
| `Paused` | Debug Session Paused |

Menu items and toolbar buttons are enabled/disabled by the ActionRegistry observing these transitions.

## GLSL Shaders

- GLSL-specific knowledge (parameters, compilation errors, hover) complements the editing and compiling Commands. It is backed by `glslang` embedded in the Language Server (rather than an external process), which simplifies distribution. The embedded `glslang` is exposed as a **GLSL Language Server** process implementing the **Language Server Protocol** (LSP).
- GLSL `#include` resolution is handled by the Language Server using a configurable set of search paths defined per project.
- The **uniform/attribute inspector** panel (`xenoide-ui-uniform-inspector`) displays active uniforms, their types, and binding points derived from the compilation output.
- **SPIR-V inspection** is in scope: after a successful Compile, the SPIR-V Artifact can be disassembled (`spirv-cross` / `spirv-dis`) and shown in `xenoide-ui-disassembly` alongside the GLSL Source.
- The **debugging** capability is provided through the **RenderDoc API**, a vendor-neutral capture/replay mechanism across Intel, NVIDIA, and AMD hardware.
