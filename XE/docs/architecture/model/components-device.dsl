renderDeviceT = component "RenderDeviceT<Policy>" "Primary device class — specialized per policy. Dynamic: vtable dispatch. Static (N64/GCN/PS3): direct calls, BackendContext by value." "C++17 template" {
    tags "Component"
}

dispatchCore = component "DispatchCore<Policy>" "Inline forwarder wrapping BackendVTable. LTO collapses to direct calls on static builds." "C++17 template" {
    tags "Component"
}

commandBufferT = component "CommandBufferT<Policy>" "Command recorder. Dynamic: tagged byte-stream. Static (N64): encodes directly into rdpq display list." "C++17 template" {
    tags "Component"
}

policyDynamic = component "BackendPolicy_Dynamic" "Runtime fn-pointer dispatch — default on desktop/mobile." "Policy tag" {
    tags "Component" "Policy"
}

policyN64 = component "BackendPolicy_N64_RDP" "Static dispatch for libdragon rdpq." "Policy tag" {
    tags "Component" "Policy"
}

policyGCN = component "BackendPolicy_GCN_GX" "Static dispatch for libogc GX." "Policy tag" {
    tags "Component" "Policy"
}

policyPS3 = component "BackendPolicy_PS3_GCM" "Static dispatch for PSL1GHT libGCM." "Policy tag" {
    tags "Component" "Policy"
}

platformConfig = component "xe_platform_config.hpp" "Build-system-generated using alias: RenderDevice = RenderDeviceT<SelectedPolicy>." "Generated header" {
    tags "Component"
}
