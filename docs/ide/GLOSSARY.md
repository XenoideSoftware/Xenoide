# Xenoide IDE

A domain language for describing code-editing applications: full IDEs, SDI code editors, and related tools. Only domain concepts belong here; architecture archetypes and flows live in @ARCHITECTURE.md, and relationships/invariants live in @DOMAIN-MODEL.md.

## Language

**Workspace**:
The optional top-level container of a development session; holds one or more Projects.

**Project**:
A buildable unit; references a root folder and knows its Build System, Package Manager, and produced Artifacts.

**Source**:
A source-code file; belongs to a Project or is opened standalone.

**Document**:
The in-memory unit of editing; holds content, a dirty flag, and undo/redo history, and references at most one Source by path.

**Editor**:
A pane bound to exactly one Document; holds per-pane caret and selection.

**Document Manager**:
The owner of the set of open Documents.

**Command**:
A named operation the User invokes (Open, Save, Build, Debug, Refactor).

**Event**:
A named occurrence reported by the system (BuildFinished, DiagnosticPublished, DebugPaused).

**Tool**:
Any external capability the IDE drives or integrates.

**Invoked Tool**:
A Tool that is transient; each operation is a fresh invocation that runs and returns output.

**Server Tool**:
A Tool that is long-running; has a start-to-stop lifecycle, holds internal state, and streams events.

**Tool Capability**:
A discrete ability a Tool advertises; gates the availability of Commands.

**Symbol**:
A named, addressable code element (function, class, variable, type, macro) with a location in a Source.

**Symbol Index**:
The queryable model of Symbols across a Project's Sources.

**Diagnostic**:
Information about a location in a Source — a severity, the producing Tool, and a line/column.

**Artifact**:
A file produced by a Build; kinds are determined by the language/tech stack.

**Executable**:
A runnable, debug-capable Artifact.

**Build**:
An operation that runs a Build System over a Project to produce Artifacts and Diagnostics.

**Compile**:
A Build scoped to a single Source.

**Launch Configuration**:
How to start an Executable — the artifact, arguments, environment, and working directory.

**Run**:
Starting an Executable without a debugger.

**Debug Session**:
A stateful session in which a Debugger controls a running Executable.

**Breakpoint**:
A pause location (Source + line); a conditional Breakpoint fires when a predicate is true.
