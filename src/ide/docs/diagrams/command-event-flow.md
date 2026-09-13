# Command / Event Flow — Sequence Diagrams

Mermaid `sequenceDiagram`. Two representative flows are shown: shader compilation and file-conflict resolution.

---

## 1. Shader Compile Flow

```mermaid
sequenceDiagram
    actor       User
    participant GUI        as xenoshader<br/>(GUI client)
    participant Bus        as xenoide-msgbus<br/>(ZeroMQ)
    participant LangServer as xenoshader-langserver<br/>(glslang)
    participant Diag       as xenoide-ui-build-diagnostics<br/>(panel)

    User ->> GUI : clicks CompileAction
    note over GUI : ActionRegistry checks state<br/>(must be Editing or CompileError)
    GUI ->> Bus : cmd.shader.compile  [PUSH]

    Bus ->> LangServer : cmd.shader.compile  [PULL]
    activate LangServer
    LangServer ->> LangServer : invoke glslang

    alt Compilation succeeded
        LangServer ->> Bus : compilation.succeeded  [PUB]
        LangServer ->> Bus : diagnostics.updated (empty)  [PUB]
        Bus ->> GUI : compilation.succeeded  [SUB]
        GUI ->> GUI : transition → Compiled
        Bus ->> Diag : diagnostics.updated  [SUB]
        Diag ->> Diag : clear panel
    else Compilation failed
        LangServer ->> Bus : compilation.failed  [PUB]
        LangServer ->> Bus : diagnostics.updated (errors attached)  [PUB]
        Bus ->> GUI : compilation.failed  [SUB]
        GUI ->> GUI : transition → CompileError
        Bus ->> Diag : diagnostics.updated  [SUB]
        Diag ->> Diag : populate panel with errors
    end
    deactivate LangServer
```

---

## 2. File-Conflict Resolution Flow

```mermaid
sequenceDiagram
    participant FSWatcher  as xenoshader-fswatcher
    participant Bus        as xenoide-msgbus<br/>(ZeroMQ)
    participant GUI        as xenoshader<br/>(GUI client)
    actor       User

    FSWatcher ->> Bus : file.conflict_detected  [PUB]
    Bus ->> GUI : file.conflict_detected  [SUB]
    GUI ->> User : prompt — "File changed on disk. Reload or keep in-memory version?"

    alt User chooses Reload
        GUI ->> Bus : cmd.file.reload  [PUSH]
        Bus ->> GUI : (self — FileService handles reload)
        GUI ->> GUI : reload file content into editor
        Bus ->> GUI : file.opened  [SUB]
    else User chooses Keep
        note over GUI : discard notification, resume editing
    end
```

---

## 3. Debug Session Flow

```mermaid
sequenceDiagram
    actor       User
    participant GUI   as xenoshader<br/>(GUI client)
    participant Bus   as xenoide-msgbus<br/>(ZeroMQ)
    participant RDoc  as xenoshader-rdoc<br/>(RenderDoc)

    User ->> GUI : clicks DebugAction
    note over GUI : ActionRegistry checks state == Compiled
    GUI ->> Bus : cmd.debug.start  [PUSH]
    Bus ->> RDoc : cmd.debug.start  [PULL]
    activate RDoc
    RDoc ->> RDoc : begin RenderDoc capture session
    RDoc ->> Bus  : debug.started  [PUB]
    Bus ->> GUI   : debug.started  [SUB]
    GUI ->> GUI   : transition → Debugging

    RDoc ->> Bus  : debug.paused (shader invocation hit)  [PUB]
    Bus ->> GUI   : debug.paused  [SUB]
    GUI ->> GUI   : transition → Paused

    User ->> GUI  : clicks stop / session ends
    GUI ->> Bus   : (implicit via state transition)
    RDoc ->> Bus  : debug.stopped  [PUB]
    Bus ->> GUI   : debug.stopped  [SUB]
    GUI ->> GUI   : transition → Compiled
    deactivate RDoc
```
