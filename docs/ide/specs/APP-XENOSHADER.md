# XenoShader

## Disclaimer

NOTE: This project is a test of an AI-assisted development methodology: give the AI several architectural constraints to generate application components that are cross-functional, low-coupled, highly reusable, and that support one or more *features* (a feature being the composition of several *components* plus ad-hoc glue). After several iterations the glue should take a stable form.

## Introduction

XenoShader is a desktop application for *editing*, *building*, *previewing*, and *inspecting* **GLSL shader programs**. It does **not** attempt CPU-style debugging: GPUs do not step through source lines. Instead, live *preview* and a scriptable *inspection* overlay replace the traditional debugger.

In the domain model (@DOMAIN-MODEL.md), XenoShader is a **Document Manager** restricted to a single **Project** (a **Shader Program**) at a time, holding 1..N open **Document**s, each shown in an **Editor**.

## Domain Instantiation

The domain entities (@GLOSSARY.md) map onto XenoShader as follows:

| Domain Entity | XenoShader Instantiation |
|---|---|
| **Workspace** | Deferred. One **Project** is open at a time; multi-project sessions belong to the fuller IDE. |
| **Project** | A **Shader Program**: a root folder + a manifest (`xshader.json`) declaring stage `Source`s, uniform defaults, texture bindings, include search paths, and a default primitive. |
| **Source** | A GLSL stage file (`.vert`, `.frag`, `.comp`), the ChaiScript inspect file (`.chai`), or an include file. |
| **Document** | The in-memory unit of editing (content, dirty flag, undo/redo). |
| **Editor** | A pane bound to exactly one `Document`; the code editor and the ChaiScript editor are both `Editor`s over their respective `Document`s. |
| **Document Manager** | Owns the open `Document`s; a tabbed multi-document editor with one active `Document`. |
| **Artifact** | A **Shader Program** Artifact: a bundle of per-stage SPIR-V modules plus reflection data (active uniforms, attributes, binding points). |
| **Build** | Compiles all stages in the manifest via the Language Server. |
| **Compile** | A single-`Source` `Build` (fast diagnostics for one stage). |
| **Launch Configuration** | How to run the preview: uniform values, texture bindings, selected primitive, and GL context version/profile. |
| **Run** | Starting the preview render loop (GPU execution) from a `Launch Configuration`. |
| **Diagnostic** | An error/warning/note at a `Source` location, produced by `glslang`, the GPU driver, or the ChaiScript runtime. |

There is **no** `Debug Session`, `Debugger`, or `Breakpoint` in XenoShader: those entities are simply not instantiated.

## Tools

XenoShader drives two external Tools (@GLOSSARY.md):

| Tool | Kind | Role |
|---|---|---|
| `xenoshader-langserver` — GLSL Language Server | Server Tool | Embeds `glslang`. Provides LSP, offline validation, `#include` resolution, hover, go-to-declaration, and **uniform reflection**. Compiles stages to SPIR-V and reports Diagnostics. |
| `xenoshader-fswatcher` — Filesystem watcher | Server Tool (File Watcher) | Monitors the Project's Sources; emits `file.conflict_detected`. |

Each Tool advertises **Tool Capabilities**, which gate the available Commands:

| Tool | Advertised Capabilities |
|---|---|
| `xenoshader-langserver` | `hover`, `go-to-declaration`, `compile`, `build`, `uniform-reflection`, `include-resolution`, `completion` |
| `xenoshader-fswatcher` | `conflict-detection` |

> The **preview is deliberately *not* a Tool**. It is a concrete `View` + `Presenter` living on the UI main thread (see below), because it depends directly on the native UI toolkit and drives a native graphics API to display data.

## Software Architecture

### Processes

Multi-process from the start. The GUI is the client; background capabilities are server processes, communicating over a **Message Bus** decoupled from the GUI event loop.

| Process | Role |
|---|---|
| `xenoshader` | GUI client (Qt 6), hosting all Views/Presenters, including the preview. |
| `xenoshader-langserver` | GLSL Language Server (glslang). |
| `xenoshader-fswatcher` | Filesystem watcher. |

### Message Bus

Typed messages over a bus implemented in `xenoide-msgbus`, the only component that depends directly on ZeroMQ and protobuf. Everything else depends on the typed C++ service API, keeping the transport swappable and the rest testable without a running ZeroMQ instance.

| Layer | Responsibility | Technology |
|---|---|---|
| Transport | Moving bytes between processes | ZeroMQ (`zeromq`) |
| Serialization | Encoding/decoding messages | Protocol Buffers (`protobuf`) |
| Message Bus | Routing, subscription, request/reply | `xenoide-msgbus` |
| Service API | Domain-typed C++ interface | Thin wrappers over the bus |

The preview renderer is an **in-process worker-thread service in the GUI process** (the architecture's "hybrid concurrency"), not a bus participant. It is still a first-class `Component`/`View` with a lifecycle.

### Messages

The bus carries two kinds of Message (@ARCHITECTURE.md):

- **Command** — an imperative instruction consumed by exactly one handler (PUSH/PULL).
- **Event** — a declarative announcement broadcast to all subscribers (PUB/SUB).

**Actions** are UI-layer objects bridging user intent and Commands: each has a visual representation, an enabled/disabled state driven by the application state machine, and emits exactly one Command when activated. State enforcement happens at the Action layer, so the bus never drops messages based on state.

#### Actions

| Action | Command | Valid States |
|---|---|---|
| `FileNewAction` | `cmd.file.new` | `Idle`, `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `FileOpenAction` | `cmd.file.open` | `Idle`, `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `FileSaveAction` | `cmd.file.save` | `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `FileSaveAsAction` | `cmd.file.save_as` | `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `ExitAction` | `cmd.app.exit` | *(all)* |
| `UndoAction` | `cmd.edit.undo` | `Editing` |
| `RedoAction` | `cmd.edit.redo` | `Editing` |
| `CutAction` | `cmd.edit.cut` | `Editing` |
| `CopyAction` | `cmd.edit.copy` | `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `PasteAction` | `cmd.edit.paste` | `Editing` |
| `GoToDeclarationAction` | `cmd.edit.go_to_declaration` | `Editing`, `Compiled`, `CompileError`, `Previewing` |
| `CompileAction` | `cmd.shader.compile` | `Editing`, `CompileError` |
| `BuildAction` | `cmd.shader.build` | `Editing`, `CompileError` |
| `RunAction` | `cmd.preview.run` | `Compiled` |
| `StopAction` | `cmd.preview.stop` | `Previewing` |
| `PauseAction` | `cmd.preview.pause` | `Previewing` |
| `SetPrimitiveAction` | `cmd.preview.set_primitive` | `Compiled`, `Previewing` |
| `SetContextAction` | `cmd.preview.set_context` | `Compiled`, `Previewing` |
| `SetUniformAction` | `cmd.uniforms.set` | `Compiled`, `Previewing` |

Actions are registered in the **ActionRegistry**, which observes state transitions and enables/disables the relevant Actions in bulk.

#### Commands

| Command | Description |
|---|---|
| `cmd.file.new` | Discard the current Document, reset to a blank (unsaved) Document |
| `cmd.file.open` | Open a file picker and load a Source into a Document |
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
| `cmd.shader.compile` | Compile the current Source (a single-`Source` Build) via the Language Server |
| `cmd.shader.build` | Build the whole Shader Program (all manifest stages) via the Language Server |
| `cmd.preview.run` | Start the preview render loop (GPU execution) from the Launch Configuration |
| `cmd.preview.stop` | Stop the preview render loop |
| `cmd.preview.pause` | Pause/resume the preview render loop |
| `cmd.preview.set_primitive` | Select the preview geometry primitive |
| `cmd.preview.set_context` | Select the OpenGL context version/profile used by the preview |
| `cmd.uniforms.set` | Set a uniform value in the Launch Configuration |

#### Events

Events emitted by services; no Action maps to these directly:

| Event | Emitted by | Description |
|---|---|---|
| `file.opened` | FileService | A Source was loaded into a Document |
| `file.saved` | FileService | The Document was written to its Source on disk |
| `file.conflict_detected` | `xenoshader-fswatcher` | The Source on disk changed while the Document holds unsaved modifications |
| `compilation.succeeded` | `xenoshader-langserver` | A single-Source Compile produced valid SPIR-V |
| `compilation.failed` | `xenoshader-langserver` | A single-Source Compile produced errors |
| `build.succeeded` | `xenoshader-langserver` | The program Build produced a valid Shader Program Artifact (SPIR-V bundle + reflection) |
| `build.failed` | `xenoshader-langserver` | The program Build produced errors |
| `diagnostics.updated` | `xenoshader-langserver` | A new set of Diagnostic items is available |
| `preview.started` | Preview Presenter | The render loop is running |
| `preview.stopped` | Preview Presenter | The render loop ended |
| `preview.paused` | Preview Presenter | The render loop is paused |
| `preview.resumed` | Preview Presenter | The render loop resumed |
| `script.compiled` | Preview Presenter (ChaiScript) | The inspect script compiled/loaded successfully |
| `script.error` | Preview Presenter (ChaiScript) | The inspect script failed to compile or threw at runtime |

### Diagnostics

All diagnostic items share the domain **Diagnostic** shape (@DOMAIN-MODEL.md):

```
Diagnostic {
  source_path   // Source location
  line
  column
  severity      // NOTE, WARNING, ERROR
  message
  tool          // "glslang", "driver", "chaiscript", "fswatcher"
}
```

This avoids duplication and lets `xenoide-ui-build-diagnostics` consume diagnostics from any source uniformly.

There are **three** diagnostic producers in XenoShader:

- **`glslang`** (offline): produced by `Compile`/`Build` in the Language Server.
- **`driver`** (runtime): produced when the preview feeds a program to the GPU driver under the selected context, and the driver rejects it.
- **`chaiscript`** (runtime): produced when the inspect script fails to compile or throws.

### Application State Machine

XenoShader's concrete state machine is composed from the domain building blocks (`Document.dirty`, `Build.status`):

| State | Meaning (domain-derived) |
|---|---|
| `Idle` | No Document open |
| `Editing` | Document open, not yet compiled |
| `Compiled` | Last `Build` Succeeded |
| `CompileError` | Last `Build` Failed |
| `Previewing` | The preview render loop is running (entered via `Run`, gated on `Compiled`) |

While `Previewing`, each successful `Build` live-reloads the render; a failed `Build` transitions back to `CompileError` while the preview keeps rendering the last-good program. Menu items and toolbar buttons are enabled/disabled by the ActionRegistry observing these transitions.

## Features

Features are specified by naming the concrete archetypes they compose (@ARCHITECTURE.md). Each feature is a bundle of UI `Components` + `Services` + `Commands` delivering a workflow.

### F1 — Document Management (File)

Open, create, save, and reload `Document`s; track dirty state.

- **Actions/Commands**: `FileNewAction`, `FileOpenAction`, `FileSaveAction`, `FileSaveAsAction` → `cmd.file.*`, `cmd.app.exit`.
- **Components**: `xenoide-ui-codeeditor` (View + Presenter).
- **Services**: `FileService` (emits `file.opened`, `file.saved`), `DocumentManager`.
- **Events**: `file.opened`, `file.saved`, `file.conflict_detected`.

### F2 — Code Editing

Text editing with undo/redo and clipboard, plus GLSL-aware navigation.

- **Actions/Commands**: `UndoAction`, `RedoAction`, `CutAction`, `CopyAction`, `PasteAction`, `GoToDeclarationAction` → `cmd.edit.*`.
- **Components**: `xenoide-ui-codeeditor`.
- **Services**: `xenoshader-langserver` (hover, go-to-declaration).
- **Tool Capabilities**: `hover`, `go-to-declaration`, `completion`.

### F3 — Project Explorer

Navigate the open Shader Program's Sources.

- **Components**: `xenoide-ui-project-explorer` (View + Presenter).
- **Services**: `DocumentManager`.
- **Events**: `file.opened`.

### F4 — Compile & Build

Validate a single stage or the whole program; produce the Shader Program Artifact and Diagnostics.

- **Actions/Commands**: `CompileAction` → `cmd.shader.compile`; `BuildAction` → `cmd.shader.build`.
- **Components**: `xenoide-ui-build-diagnostics` (View + Presenter).
- **Services**: `xenoshader-langserver` (compile, build, uniform-reflection).
- **Events**: `compilation.succeeded`, `compilation.failed`, `build.succeeded`, `build.failed`, `diagnostics.updated`.
- **Tool Capabilities**: `compile`, `build`, `uniform-reflection`, `include-resolution`.

### F5 — Preview (Run)

Start/stop/pause the render loop; select the geometry primitive and the GL context version/profile.

- **Actions/Commands**: `RunAction`, `StopAction`, `PauseAction`, `SetPrimitiveAction`, `SetContextAction` → `cmd.preview.*`.
- **Components**: `xenoide-ui-preview` (View + Preview Presenter — primary port, UI main thread), `xenoide-ui-launch-config` (View + Presenter).
- **Services**: none external; the render loop runs in-process.
- **Events**: `preview.started`, `preview.stopped`, `preview.paused`, `preview.resumed`.

### F6 — Launch Configuration

Edit uniform values, texture bindings, and the selected primitive for the next Run.

- **Actions/Commands**: `SetUniformAction` → `cmd.uniforms.set`; `SetPrimitiveAction` → `cmd.preview.set_primitive`.
- **Components**: `xenoide-ui-launch-config` (form over uniforms/textures/primitive), `xenoide-ui-uniform-inspector` (read-only reflection view).
- **Services**: `xenoshader-langserver` (uniform-reflection).
- **Events**: `build.succeeded` (populates the uniform list).
- **Tool Capabilities**: `uniform-reflection`.

### F7 — Inspection (ChaiScript overlay)

Run a per-frame ChaiScript that reads back GPU data, computes metrics, and draws HUD overlays.

- **Actions/Commands**: the script is a `Source` edited in `xenoide-ui-codeeditor`; saving reloads it.
- **Components**: `xenoide-ui-preview` (hosts the ChaiScript interpreter in its Presenter).
- **Services**: ChaiScript runtime embedded in the Preview Presenter.
- **Events**: `script.compiled`, `script.error`.

### F8 — SPIR-V Disassembly

Show the compiled output of the current Build.

- **Components**: `xenoide-ui-disassembly` (View + Presenter).
- **Services**: `xenoshader-langserver` (produces SPIR-V).
- **Events**: `build.succeeded`.

## Preview & Inspection

### Render loop

- The preview renders **continuously** (per frame), with a configurable frame rate and pause. A `time` uniform is advanced each frame, enabling animated shaders.
- On each successful `Build`, the render loop live-reloads the Shader Program; on failure it keeps the last-good render.

### GPU feed (hybrid)

The preview feeds the GPU the program form that the **selected context** supports:

- If the selected OpenGL context supports SPIR-V ingestion (`ARB_gl_spirv`, core in GL 4.6), the preview feeds the per-stage SPIR-V produced by `Build`.
- Otherwise, the preview feeds the GLSL source directly, and the driver compiles it under the selected context.

This allows XenoShader to exercise the GPU actually available on the machine, and to test the same source across GL versions/profiles via the context selector.

### Context version/profile selector

A selector on the preview `View` lets the user pick the target OpenGL context version/profile (e.g. `4.6 core`, `3.3 core`, `compatibility`, `ES`). Changing it recreates the preview's GL context and re-runs the program under that target.

### Prebuilt primitives

The user selects a prebuilt 3D primitive for the preview, starting with a full-screen quad and growing to sphere and other primitives. The primitive supplies the geometry (positions, normals, UVs) that the program's stages consume; the selected primitive is part of the `Launch Configuration`.

### Uniform & texture inspection

- The `Launch Configuration` `View` presents the program's active uniforms and their data types, populated by `uniform-reflection` from the Language Server (not hand-authored). The user edits values here.
- The `xenoide-ui-uniform-inspector` is a read-only mirror of the same reflection data.
- Textures are image files (e.g. PNG/KTX) referenced by the manifest, bound to sampler uniforms by the `Launch Configuration`.

### ChaiScript overlay

- The inspect script is a `.chai` `Source` in the Project, editable in XenoShader's editor.
- It runs **continuously, per frame**, inside the Preview Presenter (UI main thread).
- It can read back framebuffer regions, active uniform values, and frame timing; compute metrics/statistics; and draw HUD text/overlays on the viewport.
- Saving the script recompiles/reloads it; compile/runtime errors are emitted as `Diagnostic`s (`tool = "chaiscript"`).

## Invariants

- A `Document` references at most one `Source`; an unsaved Document references none.
- A `Command` is invocable only when every required `Tool Capability` is advertised by the configured `Tool`.
- A `Diagnostic` is produced by exactly one `Tool`/producer and located in exactly one `Source`.
- `Run` (`cmd.preview.run`) is available only in state `Compiled`.
- The preview is a `View` on the UI main thread; it is never a bus participant.
