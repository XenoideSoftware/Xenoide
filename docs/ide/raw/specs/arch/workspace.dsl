workspace "XenoIDE" "IDE for Native App and Shader development." {
    !identifiers hierarchical

    model {
        ss = softwareSystem "XenoIDE" {
            ui = container "Native UI" {
                document_manager = component "Document Manager View"
                document_manager_presenter = component "Document Manager Presenter"
                
                folder_explorer = component "Folder Explorer View"
                folder_explorer_presenter = component "Folder Explorer Presenter"

                zmq_port = component "I/O ZeroMQ Port"

                document_manager -> document_manager_presenter "asdas"
                folder_explorer -> folder_explorer_presenter "asdas"

                document_manager_presenter -> zmq_port "asdas"
                folder_explorer_presenter -> zmq_port "asdas"
            }

            core = container "IDE Core process" {
                zmq_port = component "I/O ZeroMQ Port"
                component1 = component "Orchestrator"
                lsp_port = component "LSP Port"
                fs = component "FileSystem Abstraction"
                ws = component "Workspace Model"

                zmq_port -> component1 "asdasd"
                component1 -> lsp_port "asdsad"
                component1 -> fs "asdsad"
                component1 -> ws "asdsad"
            }

            lsp_cpp = container "C++ Language Server Protocol"
            lsp_glsl = container "GLSL Language Server Protocol"
            re_cpp = container "C++ refactor engine"
        }

        ss.ui -> ss.core  "Send App Action" "ZeroMQ"
        ss.core -> ss.ui  "Notify App Action" "ZeroMQ"
        ss.core -> ss.lsp_cpp  "Uses" "lsp"
        ss.core -> ss.lsp_glsl  "Uses" "lsp"
        ss.core -> ss.re_cpp  "Uses" "lsp"
    }

    views {
        systemContext ss "Diagram1" {
            include *
        }

        container ss "Diagram2" {
            include *
        }

        component ss.ui {
            include * 
            autoLayout lr
        }

        component ss.core {
            include * 
            autoLayout lr
        }

        styles {
            element "Element" {
                strokeWidth 7
                shape roundedbox
            }
            element "Software Engineer" {
                shape person
            }
            element "Boundary" {
                strokeWidth 5
            }
            relationship "Relationship" {
                thickness 4
            }
        }
    }

    configuration {
        scope softwaresystem
    }
}
