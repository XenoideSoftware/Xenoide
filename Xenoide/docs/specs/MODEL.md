
Some things identified that needs to be taken a form

# Actions. 

Something that happens in the application, triggered by the end user or programmatically. It is sent trough a bus and processed asynchronously by several Application Features.
List of actions:

- file_new
- file_open
- file_save
- file_saveas
- file_close

# UIs: 
An User Interface is a Window displayed to the end user to manipulate one or more Features, or to retrieve information. Some examples:

- ui_callstack
- ui_editor
- ui_mainwindow
- ui_output
- ui_terminal
- ui_filesavedialog

Some UI are root (they don't have any other parents), and others are part of a greater UI. For example, ui_editor, ui_output and ui_terminal are attached internally to ui_mainwindow, but ui_filesavedialog not.

- UI Component: A reusable UI Component that can be integrated in other Components to create more complex UI logic.

# Feature: 

A collection of UIs, Background Services, processed and emitted custom actions that implement a specific workflow that the user does to acomplish a task.

A list of Features can be found in the [csv/features.csv] file.

- Feature Categories 
- Background Service: It is a process that runs continuously in the background, reacting to actions in the background. Some examples are clangd wrapper, and the Filesystem Watcher.
