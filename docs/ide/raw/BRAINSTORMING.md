 
## Xenoide C++ refactoring engine:

- Although we still want to integrate AI features, that should not be the focus of the IDE. AI should be another available tool, just like version control, refactor, code navigation, etc. 

- We can design the architecture from the front up considering AI tooling.Refactoring engineMove function impl from header to sourceMove set of declarations into a new file / existing fileRefactor engine that allows to specify custom refactorings using a SQL-like language.

- This does require knowledge in the LLVM / Clang toolchain -> worth exploring deeply, may unlock several advanced features, like implement code hot-reloading, the refactoring engine described before, and many others.

## Miscellaneous
- UI Style:An IDE that does not sits in the way  will enable Quick iterations.

- IDE should not block the code -> minimize modal dialogs, use attached tool windows where possible. Should signal background processes.

- Ability to perform remote development / debugging on a Remote server, Dev container, etc.

- "Assign" a terminal window per subfolder in a workspace (useful for Monorepos) 

- Visual Studio don't get the Conan runenv, but CLion does.

- Clion goes insanely slow on Linux -> High performance requirement.

- Other things create the business capability map to help organize features and map them to software components that support them. Maybe we should use Markdown along with Metadata to enable tooling around it?, like diagram generation.

- Fix the CI / CD on GitHub- Change the App colour theme / source code theme on-demand or automatically, due to screen reflection, environment light / night light, etc.

- Generate programmatically Icons for the App (like file types, appending tiny C++ texts over here and there, disabled colours, theming, etc.). Maybe one idea is to generate SVG files, them "compile them" to various resolutions, formats, etc.

- Xenoide tries to bring back the "lost art" of native, desktop app development, to use the hardware to its most potential.

- Create a file system indexer / watcher component / module to quickly search for files: This should use native APIs whenever possible to achieve maximum performance.

- Xenoide should include several products: Editor (to quickly open configuration files), IDE, Playground (to quickly create proof of concepts, include utility gist utility code, include code shared by others, like utility function, class etc. 

- Automatically compute metrics like cyclomatic complexity, etc).

- See how can we enforce linting/rules to high-level components, like CMake target namimg conventions, patterns in C++ code, and so on.

- Built-in support for Mono repos: Introduce facilities to develop inter-related projects in the same session, simultaneously.

- Learning Mode: Display a tutorial or documentation in the same window as the code in order to learn something new.

- When running an action (like installing a plugin), don't hide the logs, but show them. This can help in case of issues.

- All actions in the UI should be searchable.

- In-IDE browser of source code-generated Doxygen documentation 

- Generate a "Survival guide": generate a document from "what it does" functionality to a function or class, to guide developers and LLMs to use the previously built functions instead of creating something from scratch.

- Given Conan packages: Add the ability to modify a Conan recipe in a mono repository scheme.

- Playground Mode: Quickly create a project to test a library. Use an LLM to quickly generate a main file to see it.

- Don't allow editing of files outside the project.

## Xenoide C++ Refactoring Engine

- Although we still want to integrate AI features, that should not be the focus of the IDE. AI should be another available tool, just like version control, refactor, code navigation, etc.

- We can design the architecture from the ground up considering AI tooling.

- **Refactoring engine** — operations to support:
  - Move function implementation from header to source.
  - Move a set of declarations into a new or existing file.
  - Define custom refactorings using a SQL-like language.

- This does require knowledge of the LLVM / Clang toolchain — worth exploring deeply, as it may unlock several advanced features: code hot-reloading, the refactoring engine described above, and many others.

## Competitive Observations

- Visual Studio does not pick up the Conan `runenv`, but CLion does. → **Requirement**: preserve Conan `runenv` in the IDE's run/debug launcher.

- CLion goes very slow on Linux. → **Requirement**: high-performance indexing and UI; never block the main thread with I/O or analysis work.

## Miscellaneous Feature Ideas

- **UI style**: an IDE that does not get in the way will enable quick iterations.

- The IDE should not block coding: minimize modal dialogs, use attached tool windows where possible, and always signal background processes.

- Ability to perform remote development / debugging on a remote server, dev container, etc.

- Assign a terminal window per subfolder in a workspace (useful for monorepos).

- Create a business capability map to help organize features and map them to the software components that support them. Consider using Markdown with embedded metadata to enable tooling around it, like automatic diagram generation.

- Fix the CI / CD pipeline on GitHub.

- Change the app colour theme / source code theme on demand or automatically, based on screen reflection, ambient light, night-light mode, etc.

- Generate icons programmatically (file types, appended C++ text labels, disabled-state colours, theming, etc.). One approach: generate SVG sources and "compile" them to various resolutions and formats as a build step.

- Xenoide tries to bring back the "lost art" of native desktop app development — use the hardware to its full potential.

- Create a filesystem indexer / watcher component to quickly search for files; use native OS APIs wherever possible for maximum performance.

- Xenoide should encompass several products: **Editor** (quickly open configuration files), **IDE**, and **Playground** (quickly create proof-of-concept projects, include utility snippets, share utility functions/classes with others).

- Automatically compute code metrics such as cyclomatic complexity.

- Enforce linting rules on high-level components: CMake target naming conventions, patterns in C++ code, etc.

- Built-in support for monorepos: introduce facilities to develop inter-related projects in the same session simultaneously.

- **Learning Mode**: display a tutorial or documentation alongside the code in order to learn something new in context.

- When running an action (like installing a plugin), show the logs rather than hiding them — helps diagnose failures.

- All actions in the UI should be searchable.

- In-IDE browser for Doxygen documentation generated from source code.

- Generate a **"Survival Guide"**: a document derived from the "what it does" descriptions of functions and classes, to help developers and LLMs reuse existing code rather than re-implementing it from scratch.

- Add the ability to modify a Conan recipe within a monorepo scheme.

- **Playground Mode**: quickly create a project to test a library; use an LLM to generate an initial `main` file to get started.

- Do not allow editing files outside the project.
