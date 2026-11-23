
# Xenoide

Delightful and fun IDE for native, high performance applications, such as App, Games, Console, Shader and Compute development.



## Example Use Cases
- Desktop Applications
- Games
- Engines 
- Applications for Systems / Retro Consoles
- Broodwar bots
- Data Processing Applications
- Remote Development
- Mobile Specialized Applications
- Cross-Platform Applications

## Functional Requirements

### 1. Project Management
- CMake-based project support
- Workspace and project management
- Build process orchestration

### 2. Code Editor
#### 2.1 Programming Language Support
- C/C++
- CMake
- GLSL
- OpenCL C
- Python
- Assembly

#### 2.2 Syntax Highlighting
- Keywords
- Literals (numeric, strings)
- Operators
- Identifiers (classes, variables, methods)
- Comments
- Documentation comments

#### 2.3 Intellisense Support
- Autocomplete for class members and functions
- Function information (parameters, return value, documentation)
- Identifier information and cross-references
- Suggestions list while typing

#### 2.4 Refactoring Support
- Rename symbol
- Extract method
- Implement interface
- Extract interface
- Generate implementation (C++)
- Refactoring engine that allows to implement custom refactoring "queries", by seeing the code base as a database.

#### 2.5 Editor Configuration
- Tab configuration (spaces count, TAB vs SPACE character)
- Code style configuration per language (color, font name, size, style)

### 3. Debugging Support
- Breakpoints management
- Start, Continue, Stop, and Pause debugging
- Step-by-step execution (next line, step out, step into)
- Variables inspection
- Memory addresses visualization
- Disassembly view
- Local variables and parameters inspection

### 4. User Interface
#### 4.1 Layout Structure
- Center section (document editors)
- Footer section
- Left Panel
- Right Panel
- Detachable editor windows (multi-monitor support).

#### 4.2 Support Views
- IDE Logs (internal operations for developers and plugin authors)
- Output Window (standard output of current process)
- Project Explorer (filesystem and CMake structure)
- Class Explorer (code elements and relationships)
- Code Structure View (declarations tree in the current source code editor)

### 5. Plugin System
- Plugin-based architecture
- Plugin Manager interface:
  - View available plugins
  - Install/Uninstall plugins
  - Query plugin information (status, version)
  - Manage plugins (start, stop, enable, disable)
- Plugin implementation:
  - C++ for performance-critical functionality
  - Scripting languages (Lua, ChaiScript) for utilities and code generation

### 6. Profiler Mode
- Performance profiling capabilities (to be defined)

## Non-Functional Requirements

### Cross Platform Support

#### Primary Platforms
- Windows 10, 11
- Linux
- macOS

#### Secondary Platforms
- Unix-based (FreeBSD, OpenBSD, NetBSD, etc)
- Legacy 9x-based Windows (95, 98, ME)
- Legacy NT-Based Windows (XP, Vista, 7, 10, 11)

### Native User Experience
We have considered using native UI toolkits per platform for seamless integration and low resource usage, but we have decided to use *wxWidgets* as our cross-platform UI toolkit. The reason for this decision is that it is a cross-platform UI toolkit that is easy to use, have many different controls already implemented and it is easy to extend. The best part is that they already are using native widgets whenever possible.

However, we don't want let wxWidgets "corrode" the codebase: UI and bussiness logic must be strictly separated, so that in the future we could change the UI framework or add additional UI interfaces to support additional platforms, and support remote development.

### Architecture Quality Attributes

Architectural quality attributes are important to both, drive the development process in a sustainable way, and also, create an application that is high-performant, and helps software engineers to create great products with it (in that order)

The reason for this is because we have little time available: while modifying the codebase we should ensure that we are not introducing bugs, but also we should not let the codebase rot progressively.

Whenever possible, each quality attribute should have associated one or more *fitness functions*, that are checked during CI/CD pipelines, to prevent the decrease in quality over the time.

#### Maintainability: 

The codebase must be *easy to understand and navigate*, *self-documenting*, *well structured*, enable the *implementation of new features quickly*.

**Easy to understand and navigate**: 

The code must have consistent formatting, with an unique naming convention, low cyclomatic complexity. Functions and methods must not be long. The code elements naming must be strict (functions and methods must start with verbs, and classes with nouns). 

**Self-documenting**: 
In a ideal world, the code *should* read as pseudo-code

#### Responsiveness: 



#### Extensibility: 
Via plugins with seamless integration

**Enablers**: Dependency Injection / Inversion of Control, Scripting languages

#### Testability: 
Each component must be testable

**Enablers**: Dependency Injection / Inversion of Control, Unit Testing, Mocking

#### Multi-process architecture: 
Core IDE and services separation for high responsiveness, isolation and security

### Technology Support
- Support for multiple toolchains (CMake, Conan, vcpkg, etc.)
- Support for multiple programming languages in the same project
- Support for dependencies (libraries and other projects)
