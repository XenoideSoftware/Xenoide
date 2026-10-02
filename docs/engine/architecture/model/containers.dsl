xeRenderer = softwareSystem "XE Renderer" "Multi-tier, multi-backend rendering middleware for the XE engine." {
    tags "XE"

    publicApi = container "Public API" "RenderDevice, CommandBuffer, all *Desc / *Handle public types." "C++17 headers" {
        tags "Container"
    }

    dispatch = container "Dispatch" "RenderDeviceT<Policy>, DispatchCore<Policy>, CommandBufferT<Policy> — static or dynamic call routing." "C++17 templates" {
        tags "Container"
    }

    backendAbi = container "Backend ABI" "BackendVTable, BackendContext, factory signatures — the C ABI boundary between dispatch and backends." "C ABI" {
        tags "Container"
    }

    backends = container "Backends" "Concrete backend implementations: GL Legacy, GL4, D3D11, Metal, GLES2/3, N64 RDP, GCN GX, PS3 GCM, SoftBuiltin, SoftMesa, Null." "Per-backend TU" {
        tags "Container"
    }

    caps = container "Capabilities" "SystemCaps (CPU/GPU/Memory/Platform), BackendRegistry — hardware introspection and backend enumeration." "C++17" {
        tags "Container"
    }

    services = container "Services" "XeServices — allocator, log sink, profiler hook, assert handler, alloc tracker." "C ABI callbacks" {
        tags "Container"
    }

    nativeExt = container "Extension Headers" "xe/native/* opt-in headers providing typed access to underlying API objects (GLuint, GXTexObj, rsxBuffer, etc.)." "C++17 opt-in headers" {
        tags "Container"
    }

    assetCompiler = container "Asset Compiler" "xe-asset-compiler CLI — build-time tool converting source assets to platform-native .xemesh / .xetex binaries." "Standalone executable" {
        tags "Container" "BuildTime"
    }

    assetRuntime = container "Asset Runtime Loader" "Thin mmap/DMA loader issuing create_* calls from compiled .xemesh / .xetex files." "C++17" {
        tags "Container"
    }
}
