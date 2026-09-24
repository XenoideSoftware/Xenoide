workspace "XE Renderer" "C4 architecture model of the XE Engine rendering middleware." {

    model {
        !include model/people.dsl
        !include model/systems.dsl
        !include model/containers.dsl

        !ref xeRenderer {
            !ref dispatch {
                !include model/components-device.dsl
            }
            !ref backends {
                !include model/components-backends.dsl
            }
            !ref assetCompiler {
                !include model/components-asset.dsl
            }
        }

        !include model/relationships.dsl
    }

    views {
        !include views.dsl
    }

}
