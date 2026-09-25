# CommandBuffer API — Initial Interface Design

Companion to `rendering-subsystem-spec.md`. Proposes the first-cut interface for the `CommandBufferT<Policy>` surface (spec §11.4, §12) that is currently absent from `src/libxe-graphics/src/xe/render/`.

## Context

`src/libxe-graphics/` is mid-redesign. The spec at `rendering-subsystem-spec.md` (rev 0.7) describes a policy-parameterized rendering middleware whose central per-frame work item is `CommandBufferT<Policy>` — a **thread-safe record surface** that crosses the backend boundary exactly once per submit, not once per draw (spec §9.2, §11.4).

What the repo already has in `src/xe/render/`:

- `RenderDeviceBackendVTable` with function-pointer slots for resource create/destroy + beginFrame/endFrame/present (`RenderBackend.h`).
- `detail::HandleT<HandleType Tag, SubEnum>` — typed 32-bit handles with `subType:4 | gen:8 | reserved:4 | index:16` layout (`types.h`). Aliases: `BufferHandle`, `TextureHandle`, `ShaderHandle`, `VertexLayoutHandle`, `PipelineHandle`, `GeometryHandle`.
- Descriptor conventions: `*Descriptor` suffix, POD, borrow pointers, no allocations. `tl::expected<T, BackendError>` error model.
- GL3 backend partial (`backend/glcore3-api.h`).

What is **missing** — and what this design delivers:

- No `CommandBuffer` of any kind.
- No `submit*` slot on the vtable.
- No `PrimitiveType` enum, no `SamplerHandle`/`RenderTargetHandle`/`UniformBufferHandle`.
- No `RenderDeviceT<Policy>` wrapper.

The goal is a first-cut, API-agnostic `CommandBuffer` that records state+draw commands in the caller's thread with minimal per-call overhead, plus the single backend vtable addition needed to submit them.

## Design Summary

**Encoding:** fixed-size per-opcode POD packets in a byte stream + a side payload arena for variable-length uniform / push-constant data. Recording a command is a bounds check + a `memcpy`-equivalent store + a cursor bump. Decoding is a `switch(op)` with compile-time-known packet sizes per case — no length reads, no per-field parse state.

**Policy:** `BackendPolicy_Dynamic` is the only specialization implemented in v0. Other policy tags (`BackendPolicy_N64_RDP`, `BackendPolicy_GCN_GX`, `BackendPolicy_PS3_GCM`) are forward-declared so the class-template shape is frozen. Static specializations (which encode directly into native DL format) are added later without client-code changes — the public method surface is identical across specializations.

**Error model:** sticky `RecordErrorCode` on the buffer, not `tl::expected<void, _>` per record call. Hot-path recording stays branch-light; one check at submit time decides whether to proceed. `tl::expected` is preserved only at resource-creation boundaries, matching the existing pattern.

**Arena ownership:** v0 takes `(uint8_t* stream, uint32_t streamCap, uint8_t* payload, uint32_t payloadCap)` — caller-owned memory, matching the spec's "no hidden allocations" rule and avoiding a hard dependency on the still-absent `XeAllocator`. An allocator-taking constructor is a later revision.

**Submit coupling:** one new vtable slot `submitBatch(ctx, buffers[], count)` — appended, not inserted, per spec §9.2's ABI rule. Per-buffer `submit` is a `RenderDeviceT`-level wrapper with `count=1`.

## Files to Modify / Add

| Path | Action |
|---|---|
| `src/libxe-graphics/src/xe/render/types.h` | **Extend**: add `HandleSampler / HandleRenderTarget / HandleUniformBuffer` to `HandleType` enum; add `SamplerHandle / RenderTargetHandle / UniformBufferHandle` typed aliases; add `PrimitiveType` enum; add `SamplerDescriptor` POD (full integration deferred, but the type needs to exist for `bindTexture`); add `RenderTargetDescriptor` POD (minimal). |
| `src/libxe-graphics/src/xe/render/RenderBackend.h` | **Extend**: append `submitBatch` slot to `RenderDeviceBackendVTable`. Do not reorder existing slots. |
| `src/libxe-graphics/src/xe/render/CommandBuffer.h` | **NEW**: public header — opcode enum, packet PODs, `CommandBufferBase`, `CommandBufferT<BackendPolicy_Dynamic>`, forward-declared static-policy specializations. |
| `src/libxe-graphics/src/xe/render/CommandBuffer.cpp` | **NEW**: `CommandBufferT<BackendPolicy_Dynamic>` method bodies (bounds-checked cursor bump per op). |
| `src/libxe-graphics/src/xe/render/backend/glcore3-api.cpp` | **Extend**: implement `submitBatchGL()` — decode loop switching on `CommandOp`, translating packets to Glaze/GL3 calls. Register it in the vtable populator. |
| `src/libxe-graphics/CMakeLists.txt` | **Extend**: add new source + header to the target. |
| `src/libxe-graphics/unit-test/` | **NEW**: `CommandBufferTest.cpp` — Catch2 suite covering record/submit round-trip with a mock backend capturing opcodes. |

## Interface — `xe/render/CommandBuffer.h` (proposed shape)

```cpp
#pragma once
#include <cstdint>
#include <xe/render/types.h>

namespace xe {

    // --- Policy tags. Only Dynamic is instantiated in v0. ---
    struct BackendPolicy_Dynamic {};
    struct BackendPolicy_N64_RDP {};
    struct BackendPolicy_GCN_GX {};
    struct BackendPolicy_PS3_GCM {};

    // --- Opcode set. Append-only; reordering == ABI break. ---
    enum class CommandOp : uint8_t {
        Nop = 0,
        BindPipeline,
        BindVertexBuffer,
        BindIndexBuffer,
        BindGeometry,         // XE-specific shortcut: vertex+index+layout in one bind
        BindTexture,          // (texture, sampler, unit)
        BindUniformBuffer,
        SetRenderTarget,      // default-constructed handle == swap chain
        ClearColor,
        ClearDepthStencil,
        SetViewport,
        SetScissor,
        SetPushConstants,     // payload in side arena
        SetUniform,           // payload in side arena
        SetUniformMatrix,     // payload in side arena
        Draw,
        DrawIndexed,
        DebugPush,            // label string in side arena
        DebugPop,
        DebugMarker,
        _Count
    };

    // --- Packet PODs. Every packet starts with a 4-byte header so the
    //     decoder can read the opcode at *cursor before advancing. ---
    struct alignas(4) PacketHeader { uint8_t op; uint8_t pad[3]; };

    struct PacketBindPipeline       { PacketHeader h; PipelineHandle pipeline; };
    struct PacketBindVertexBuffer   { PacketHeader h; BufferHandle buffer; uint32_t byteOffset; uint8_t slot; uint8_t pad[3]; };
    struct PacketBindIndexBuffer    { PacketHeader h; BufferHandle buffer; uint32_t byteOffset; GeometryIndexType indexType; };
    struct PacketBindGeometry       { PacketHeader h; GeometryHandle geometry; };
    struct PacketBindTexture        { PacketHeader h; TextureHandle texture; SamplerHandle sampler; uint8_t unit; uint8_t pad[3]; };
    struct PacketBindUniformBuffer  { PacketHeader h; UniformBufferHandle ubo; uint8_t binding; uint8_t pad[3]; };
    struct PacketSetRenderTarget    { PacketHeader h; RenderTargetHandle target; };
    struct PacketClearColor         { PacketHeader h; float rgba[4]; };
    struct PacketClearDepthStencil  { PacketHeader h; float depth; uint32_t stencil; };
    struct PacketSetViewport        { PacketHeader h; int32_t x, y, w, hgt; float minDepth, maxDepth; };
    struct PacketSetScissor         { PacketHeader h; int32_t x, y, w, hgt; };
    struct PacketSetPushConstants   { PacketHeader h; uint32_t payloadOffset; uint32_t sizeBytes; uint16_t slot; uint16_t pad; };
    struct PacketSetUniform         { PacketHeader h; int32_t location; uint16_t typeTag; uint16_t count; uint32_t payloadOffset; };
    struct PacketSetUniformMatrix   { PacketHeader h; int32_t location; uint16_t rows; uint16_t cols; uint32_t count; uint32_t payloadOffset; };
    struct PacketDraw               { PacketHeader h; PrimitiveType prim; uint32_t firstVertex; uint32_t vertexCount; uint32_t firstInstance; uint32_t instanceCount; };
    struct PacketDrawIndexed        { PacketHeader h; PrimitiveType prim; uint32_t firstIndex; uint32_t indexCount; int32_t vertexOffset; uint32_t firstInstance; uint32_t instanceCount; };
    struct PacketDebug              { PacketHeader h; uint32_t payloadOffset; uint32_t length; };

    // --- Recording error sticky on the buffer ---
    enum class RecordErrorCode : uint8_t {
        Ok = 0,
        StreamOverflow,
        PayloadOverflow,
        InvalidHandle,
        InvalidArgument,
        Poisoned,           // a prior record call failed; all further records are no-ops
    };

    // --- The POD surface the vtable sees. Binary-stable. ---
    struct CommandBufferBase {
        const uint8_t *streamBegin = nullptr;
        const uint8_t *streamEnd   = nullptr;
        const uint8_t *payloadBegin = nullptr;
        const uint8_t *payloadEnd   = nullptr;
        uint32_t policyTag = 0;    // compile-time constant per specialization
    };

    template <class Policy> class CommandBufferT;

    /**
     * @brief Recording surface for the dynamic-dispatch backend policy.
     *
     * Records opcodes into a caller-owned byte stream; records variable-length uniform /
     * push-constant / debug payloads into a caller-owned side arena. Thread-safe with respect
     * to other CommandBufferT instances: no shared state. Not safe for concurrent record from
     * multiple threads into the *same* buffer.
     */
    template <> class CommandBufferT<BackendPolicy_Dynamic> : public CommandBufferBase {
    public:
        CommandBufferT(uint8_t *streamBuf, uint32_t streamCap,
                       uint8_t *payloadBuf, uint32_t payloadCap) noexcept;

        // Lifecycle
        void reset() noexcept;
        bool isValid() const noexcept;
        RecordErrorCode lastRecordError() const noexcept;

        // State binds
        void bindPipeline(PipelineHandle p) noexcept;
        void bindVertexBuffer(uint8_t slot, BufferHandle b, uint32_t byteOffset = 0) noexcept;
        void bindIndexBuffer(BufferHandle b, GeometryIndexType t, uint32_t byteOffset = 0) noexcept;
        void bindGeometry(GeometryHandle g) noexcept;
        void bindTexture(uint8_t unit, TextureHandle t, SamplerHandle s) noexcept;
        void bindUniformBuffer(uint8_t binding, UniformBufferHandle u) noexcept;

        // Targets + clears
        void setRenderTarget(RenderTargetHandle rt) noexcept;
        void clearColor(float r, float g, float b, float a) noexcept;
        void clearDepthStencil(float depth, uint32_t stencil) noexcept;

        // Dynamic state
        void setViewport(int32_t x, int32_t y, int32_t w, int32_t h,
                         float minDepth = 0.0f, float maxDepth = 1.0f) noexcept;
        void setScissor(int32_t x, int32_t y, int32_t w, int32_t h) noexcept;

        // Uniforms / push constants (variable-length payload)
        void setPushConstants(uint16_t slot, const void *data, uint32_t sizeBytes) noexcept;
        void setUniform(int32_t location, const void *data,
                        uint16_t typeTag, uint16_t count) noexcept;
        void setUniformMatrix(int32_t location, const float *data,
                              uint16_t rows, uint16_t cols, uint32_t count = 1) noexcept;

        // Draws
        void draw(PrimitiveType prim, uint32_t firstVertex, uint32_t vertexCount,
                  uint32_t firstInstance = 0, uint32_t instanceCount = 1) noexcept;
        void drawIndexed(PrimitiveType prim, uint32_t firstIndex, uint32_t indexCount,
                         int32_t vertexOffset = 0,
                         uint32_t firstInstance = 0, uint32_t instanceCount = 1) noexcept;

        // Debug scopes
        void debugPush(const char *label) noexcept;
        void debugPop() noexcept;
        void debugMarker(const char *label) noexcept;

    private:
        uint32_t streamCap_ = 0;
        uint32_t payloadCap_ = 0;
        uint8_t *streamCursor_ = nullptr;
        uint8_t *payloadCursor_ = nullptr;
        RecordErrorCode err_ = RecordErrorCode::Ok;

        template <class P> P *allocPacket(CommandOp op) noexcept;
        uint32_t allocPayload(const void *src, uint32_t bytes) noexcept;
        void poison(RecordErrorCode why) noexcept;
    };

    // --- Forward declarations; bodies live in backend-owned headers. ---
    template <> class CommandBufferT<BackendPolicy_N64_RDP>;
    template <> class CommandBufferT<BackendPolicy_GCN_GX>;
    template <> class CommandBufferT<BackendPolicy_PS3_GCM>;

} // namespace xe
```

## VTable Extension — `RenderBackend.h`

Append exactly one slot at the end of `RenderDeviceBackendVTable`:

```cpp
// After 'present', added last per ABI rule (spec §9.2).
void (*submitBatch)(RenderDeviceBackendContext *,
                    const CommandBufferBase *const *buffers,
                    uint32_t count) = nullptr;
```

The decoder lives in each backend TU. The backend reads `buffers[i]->streamBegin/streamEnd`, switches on `*cursor` (opcode byte), casts the next `sizeof(Packet)` bytes to the matching POD, resolves any side-arena offsets against `payloadBegin`, and executes.

## `types.h` Additions

```cpp
// Extend HandleType enum (append only):
enum HandleType : uint32_t {
    HandleBuffer = 0x01, HandleShader, HandleTexture,
    HandleVertexLayout, HandlePipeline, HandleGeometry,
    HandleSampler, HandleRenderTarget, HandleUniformBuffer   // NEW
};

// New typed aliases:
using SamplerHandle       = detail::HandleT<HandleSampler>;
using RenderTargetHandle  = detail::HandleT<HandleRenderTarget>;
using UniformBufferHandle = detail::HandleT<HandleUniformBuffer>;

// New primitive enum (currently lives only in xe/gl/ as GL-specific):
enum class PrimitiveType : uint8_t {
    PointList, LineList, LineStrip,
    TriangleList, TriangleStrip, TriangleFan
};

// Minimal POD shells; full integration is a follow-up:
struct SamplerDescriptor { /* wrap/filter/aniso/lodBias — §13 */ };
struct RenderTargetDescriptor { /* colorFormats[], depthFormat, msaa — §6 */ };
```

The `SamplerDescriptor` body is scope-deferred; the existence of the type and handle is what unblocks `bindTexture(unit, t, s)`. Backends that don't honor samplers yet accept `SamplerHandle{}` and fall back to texture-embedded filtering.

## Migration — `GltfRenderer::render()` Translated

Current immediate-mode loop (`src/xe-gltf-view/src/GltfRenderer.cpp:105-150`) becomes:

```cpp
// Recording -- any thread, own a CommandBufferT instance
cb.reset();
cb.setRenderTarget(RenderTargetHandle{});           // default == swap chain
cb.clearColor(0.2f, 0.2f, 0.8f, 1.0f);
cb.clearDepthStencil(1.0f, 0);
cb.setViewport(0, 0, viewportW, viewportH);

cb.bindPipeline(pbrPipeline);                        // replaces useProgram + bindRenderState(capabilities)
cb.setUniformMatrix(uMvp, &mvp[0][0], 4, 4);
cb.setUniform(uSeconds, &seconds, /*float1*/ 0x01, 1);

for (const auto &mesh : meshes) {
    for (const auto &subset : mesh.primitives) {
        cb.bindTexture(0, subset.albedo, defaultSampler);
        cb.bindGeometry(subset.geometry);           // replaces VAO + VBO + IBO bind trio
        cb.drawIndexed(PrimitiveType::TriangleList, 0, subset.count);
    }
}

// Submission -- render thread only
device.submit(&cb);   // wraps submitBatch(ctx, &cb, 1)
```

Semantic change: records are deferred until `submit`; today's code issues GL immediately. The per-frame structure is otherwise unchanged.

## Why This Meets "Minimal Overhead Per Call"

- **Record path per call**: one bounds check (`cursor + sizeof(Packet) <= end`), one `*reinterpret_cast<Packet*>(cursor) = {…}` store, one cursor bump. No virtual dispatch, no `tl::expected` unwrapping, no allocation. `noexcept` throughout.
- **Dispatch crossing frequency**: once per `submit`, not once per draw (spec §9.2 rationale). A frame with 10,000 draws crosses the vtable boundary once.
- **Decode path**: fixed-size packets make per-case decode `cursor += sizeof(Packet)` — a compile-time constant — so the `switch` lowers to a jump table with no per-field length arithmetic.
- **Static-policy path (future)**: `CommandBufferT<BackendPolicy_N64_RDP>` writes rdpq words directly during recording; submit is a DMA kickoff with zero decode. The shape of the public methods is already locked in by the Dynamic specialization — no client-code change when static specializations land.
- **Handle resolution**: handles stay `uint32_t`; the backend's pool lookup is `slots[handle.index()]` + `gen == slots[..].gen` compare (already how the existing GL3 backend works per `glcore3-api.h`).

## Out-of-Scope for This Iteration

- Full `RenderDeviceT<Policy>` wrapper (the device surface remains the existing vtable + a thin factory for now; the CommandBuffer is independently usable).
- Static-policy `CommandBufferT` bodies for N64/GC/PS3 — only the specializations' forward declarations are shipped.
- `XeServices`/`XeAllocator` integration — sticky error + caller-owned buffers avoid the dependency.
- `beginRenderPass`/`endRenderPass` — deferred until the render-graph layer is designed (spec §21); flat `setRenderTarget + clear*` covers v0.
- Compute dispatch, indirect draws, copy/blit, timestamp queries — each is a future appended opcode.
- Sampler backend slots in the vtable — `SamplerHandle{}` is valid shorthand for "use texture-embedded filtering" until `create_sampler`/`destroy_sampler` land.

## Verification

1. **Compile round-trip**: `cmake --preset conan-debug && cmake --build --preset conan-debug` — new headers compile, existing targets still link (no reordered vtable slots).
2. **Unit suite** (`CommandBufferTest.cpp`): record each opcode, assert byte-stream layout matches expected `Packet*` contents; assert payload-arena offsets are valid; assert overflow poisoning halts further recording without UB.
3. **Round-trip with a capture backend**: build a test-only backend whose `submitBatch` appends each decoded opcode to a `std::vector<CommandOp>`; record a scripted sequence (`bindPipeline → bindGeometry → drawIndexed × 3`); submit; assert the captured sequence matches.
4. **GL3 end-to-end**: port `xe-gltf-view`'s draw loop to the CommandBuffer (behind an opt-in flag); load a glTF asset; confirm visual parity against the current immediate-mode path. This is the integration-level smoke test that proves the abstraction works for a real consumer.
5. **Static-analysis**: run clang-tidy on the new files against `.clang-tidy` defaults; confirm zero cppcoreguidelines / bugprone violations (existing code passes, new code must too).
