# Xenoide IDE

A domain language for describing code-editing applications: full IDEs, SDI code editors, and related tools. Only domain concepts belong here; architecture archetypes and flows live in @ARCHITECTURE.md.

## Language

**Editor**:
A surface where the User reads, modifies, and navigates a Source.

**Editor Manager**:
The owner of the set of open Editors; every Editor is associated with exactly one Source.

**Filesystem Browser**:
A view of the folder and file hierarchy of a Project.

**Source**:
A source-code file that belongs to a Project.

**Diagnostic**:
Information produced by a Tool about a specific File location and line within a Project, intended for display in a UI.

**Project**:
The workspace model; it references the solution root folder and knows its Build System, Package Manager, and generated artifacts.

**Executable**:
The binary artifact produced by a Build operation through the Build System; it optionally carries debug information and can be debugged.

**Build System**:
A toolchain that groups source files, configuration, and options in order to produce one or more Build Artifacts.

**Package Manager**:
A toolchain that fetches, compiles, and installs external libraries used as dependencies by a Project.

**Debugger**:
A tool that runs an existing Executable to follow execution, inspect runtime values and the call stack, set breakpoints, and pause execution.
