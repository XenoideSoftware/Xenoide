systemContext xeRenderer "SystemContext" "System context diagram for XE Renderer" {
    include *
    autoLayout
}

container xeRenderer "Containers" "Container diagram showing XE Renderer internals" {
    include *
    autoLayout
}

component dispatch "DispatchComponents" "Components inside the Dispatch container" {
    include *
    autoLayout
}

component backends "BackendComponents" "All backend implementations" {
    include *
    autoLayout
}

component assetCompiler "AssetPipelineComponents" "Asset compiler stages and runtime loader" {
    include *
    include runtimeLoader
    autoLayout
}

styles {
    element "Person" {
        shape Person
        background #08427B
        color #ffffff
    }
    element "XE" {
        background #1168BD
        color #ffffff
    }
    element "Container" {
        background #438DD5
        color #ffffff
    }
    element "Component" {
        background #85BBF0
        color #000000
    }
    element "External" {
        background #999999
        color #ffffff
    }
    element "BuildTime" {
        background #D4A017
        color #000000
    }
    element "Policy" {
        shape Hexagon
        background #6B8E23
        color #ffffff
    }
    element "TierMicrocode" {
        background #8B0000
        color #ffffff
    }
    element "TierTEV" {
        background #B8860B
        color #ffffff
    }
    element "TierLegacy" {
        background #2E8B57
        color #ffffff
    }
    element "TierModern" {
        background #4169E1
        color #ffffff
    }
    element "TierExtended" {
        background #6A0DAD
        color #ffffff
    }
}
