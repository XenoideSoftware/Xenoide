# ── Person → System ────────────────────────────────────────────
applicationDeveloper -> xeRenderer "Writes game code against"
contentAuthor -> xeRenderer "Provides source assets to" "Build-time"

# ── Person → Container ────────────────────────────────────────
applicationDeveloper -> publicApi "Calls RenderDevice / CommandBuffer"
contentAuthor -> assetCompiler "Commits .glb / .png / .xetexprop"

# ── Container → Container (within XE Renderer) ───────────────
publicApi -> dispatch "Delegates to"
dispatch -> backendAbi "Calls through BackendVTable"
backendAbi -> backends "Routes to active backend"
caps -> backends "Probes available backends"
assetRuntime -> publicApi "Issues create_buffer / create_texture / create_sampler / create_pipeline"
nativeExt -> backendAbi "Casts handles via get_native_handle / query_extension"
nativeExt -> backends "Accesses backend-specific native objects"

# ── Services injection (dashed — facility, not data flow) ────
services -> publicApi "Injected into" "XeServices"
services -> dispatch "Injected into" "XeServices"
services -> backends "Injected into" "XeServices"
services -> caps "Used by query_host / query_gpu" "XeServices"
services -> assetRuntime "Injected into" "XeServices"

# ── Container → External System ──────────────────────────────
backends -> gpuDriver "Issues GL / D3D / Metal / Vulkan calls"
backends -> consoleSdk "Issues rdpq / GX / GCM calls"
backends -> osWindowing "Creates context on RenderSurface"
assetCompiler -> packageRegistry "Publishes compiled .xemesh / .xetex"
assetRuntime -> packageRegistry "Reads compiled assets from"

# ── Dispatch component relationships ─────────────────────────
renderDeviceT -> dispatchCore "Owns"
dispatchCore -> commandBufferT "Submits"
platformConfig -> renderDeviceT "Aliases via using"
platformConfig -> commandBufferT "Aliases via using"
policyDynamic -> dispatchCore "Parameterizes (vtable dispatch)"
policyN64 -> renderDeviceT "Parameterizes (direct calls)"
policyGCN -> renderDeviceT "Parameterizes (direct calls)"
policyPS3 -> renderDeviceT "Parameterizes (direct calls)"

# ── Backend component → External System ──────────────────────
glLegacy -> gpuDriver "glCallList / glTexParameter / glDrawElements"
gl4 -> gpuDriver "glDraw* / glSamplerParameter / glUseProgram"
gles2 -> gpuDriver "glTexParameter / glDrawElements"
gles3 -> gpuDriver "glSamplerParameter / glDrawElements"
d3d11 -> gpuDriver "ID3D11DeviceContext calls"
metal -> gpuDriver "MTLRenderCommandEncoder"
vulkan -> gpuDriver "vkCmdDraw / vkCmdBindPipeline"
n64Rdp -> consoleSdk "rdpq_* / rspq_flush (libdragon)"
gcnGx -> consoleSdk "GX_* / GX_CallDisplayList (libogc)"
ps3Gcm -> consoleSdk "rsxDraw* / gcmSetFlip (PSL1GHT)"

# ── Asset pipeline component relationships ───────────────────
importStage -> compilerIR "Produces"
compilerIR -> processStage "Consumed by"
processStage -> compilerIR "Writes back processed data"
compilerIR -> exportStage "Consumed by"
exportStage -> packageRegistry "Outputs .xemesh / .xetex to"
runtimeLoader -> publicApi "Calls create_* with pre-baked descriptors"
