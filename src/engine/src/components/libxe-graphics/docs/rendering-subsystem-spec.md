# XE Engine — Rendering Middleware Specification
**Revision:** 0.7 — Prose-normalized; code listings replaced with implementation approach and rationale; inconsistencies resolved.
**Project:** XE Engine
**Scope:** Core rendering middleware; public API surface, design, portability strategy.

> The C4 architecture model for this subsystem lives in [`docs/architecture/`](../../../docs/architecture/) at the repository root as a Structurizr DSL workspace. See its `README.md` for how to view it.

---

## 0. Executive Summary

The XE rendering middleware is a thin, portable, low-overhead abstraction that targets a very wide tier range of graphics hardware — from fixed-function consoles of the 1990s (N64, GameCube/Wii, PS3) through modern programmable GPUs on desktop and mobile. Its central design axis is *portability without paying for what you don't use*.

The middleware is implemented as a small dispatch shell (`RenderDeviceT<Policy>`) that forwards resource creation and command submission to a backend. Backends are reached through either a function-pointer vtable (`BackendPolicy_Dynamic`, the default on desktop/mobile) or through template specialization that inlines backend calls directly (`BackendPolicy_*` for each console and for single-backend dev/CI builds). On static-policy targets, LTO collapses the dispatch to zero runtime overhead.

The renderer owns no threads, no mutexes, and no heap — allocators are injected. Exceptions and RTTI are disabled. Every diagnostic facility (logging, profiling, assertion, allocation tracking) is reached through a single injectable `XeServices` struct whose sinks are C-style function pointers. Backend-specific types never appear in the public API; a typed escape hatch (`get_native_handle` / `query_extension`) lets advanced clients reach the underlying GL/D3D/Metal/GCM objects through opt-in extension headers.

Assets are compiled offline by `xe-asset-compiler` into directly-memory-mappable, platform-native binaries (`.xemesh`, `.xetex`, `.xeanim`). The runtime loader makes one pass over the package calling `create_*` — no decoding, no conversion, no intermediate buffers. This preserves memory predictability on tight targets (N64 TMEM: 4 KB; GameCube ARAM: 16 MB) where runtime allocation size cannot exceed the file on disk by any margin.

Command buffers are the only thread-safe record surface; the device itself is single-threaded. Multiple workers fill independent command buffers; the render thread submits a batch in explicit order. No implicit synchronization — submitted order is executed order.

The spec is deliberately silent on render graphs, multi-GPU, compute dispatch, and asynchronous streaming. These live above the middleware layer and produce ordinary command buffers for this layer to execute.

---

## 1. Goals and Constraints

| Goal | Notes |
|---|---|
| **Multi-tier hardware** | Microcode (N64) → TEV (GC/Wii) → Legacy (GL1.x–2.x) → Modern (GL4/D3D11/Metal) → Extended (Vulkan/Compute) |
| **Multi-backend per platform** | User-selectable to work around driver bugs; enumerated by `BackendRegistry` |
| **API-agnostic public surface** | No GL/GX/GCM/D3D types leak beyond backend translation units |
| **Custom allocators throughout** | Critical for console targets with no OS heap; every subsystem uses `XeAllocator` |
| **Injectable diagnostics** | `XeServices`: logging, profiling, assertion, memory tracking |
| **Zero-thread renderer** | No internal threads; client owns all parallelism via `XeJob` (separate library) |
| **Static or dynamic dispatch** | `RenderDeviceT<Policy>` collapses to direct calls on single-backend targets; dynamic vtable on desktop |
| **Native handle escape hatch** | `get_native_handle` + `query_extension` for out-of-source extensibility and native optimizations |
| **Mobile: Android + iOS** | GLES2, GLES3, Vulkan (Android); Metal, GLES2/3 (iOS) |
| **Consoles** | N64, GameCube, Wii, PS3 via open homebrew SDKs (libdragon, libogc, PSL1GHT) |
| **Software fallback** | Built-in tile rasterizer (`SoftBuiltin`) + Mesa3D reference (`SoftMesa`) |
| **Maximal portability** | Win98–Win11, Linux, macOS, Android, iOS, N64, GC/Wii, PS3; 32- and 64-bit |
| **C++ subset, no virtual dispatch** | Templates, `constexpr`, fn-pointer dispatch; no exceptions, RTTI, `std::function` |
| **Glaze for GL backends** | GL Legacy and GL4 backends use the existing `Glaze` C++ wrapper internally |
| **Offline asset pipeline** | `xe-asset-compiler` produces platform-native `.xemesh` / `.xetex` binaries; runtime loader is a thin DMA-copy layer |

---

## 2. Document Conventions

Types are written in *PascalCase*. Public methods use *snake_case*; internal glue and helper free functions use *camelCase*. Enumerants are *PascalCase*. Handles are 32-bit unsigned values whose encoding is formalized in §10 — typed aliases like `BufferHandle` are documentation-only and share the same underlying type.

*Tier* always refers to a `TierLevel`. *Backend* is a concrete rendering API implementation (GL4, D3D11, GX, RDP). *Policy* is a compile-time tag selecting static or dynamic dispatch. *Client* is the engine code consuming this middleware. *Backend code* is the code implementing the vtable slots.

Struct layouts are described in prose with field-by-field intent. A companion reference implementation lives in `src/libxe-render/` and is the authoritative source for exact layouts. Code fragments appear only as isolated syntax examples where a prose description would be less clear than the syntax itself.

---

## 3. Threading Contract

### 3.1 Renderer Has Zero Internal Threads

The renderer creates no threads, owns no mutexes, and issues no atomics on the fast path. All parallelism is the client's responsibility through `XeJob` (a separate utility library sitting above the renderer in the dependency graph). The renderer does not link against `XeJob`.

**Rationale:** this constraint is load-bearing for console targets. libogc and libdragon offer no pthreads-compatible primitives, and `std::mutex` is unavailable in their C++ toolchains. Keeping the renderer thread-unaware sidesteps that entirely. On desktop, it also means no accidental contention between engine worker threads and driver threads.

### 3.2 Thread Safety Contract

| Operation | Guarantee |
|---|---|
| `CommandBufferT` recording (`draw*`, `bind_*`, `set_*`) | **Safe from any thread** — no shared state |
| `RenderDevice` resource creation (`create_*`) | **Render thread only** |
| `RenderDevice::submit()` / `submit_batch()` | **Render thread only** |
| `RenderDevice::begin_frame()` / `end_frame()` / `present()` | **Render thread only** |
| `BackendRegistry::probe()` | **One-shot, pre-device** |
| `SystemCaps::query_host()` | **Safe from any thread** — subject to the reentrancy of the injected `XeServices` sinks |

`SystemCaps::query_host()` is safe from any thread *provided the injected `XeServices` sinks are themselves safe from any thread*. The default sinks are fully reentrant; if a client installs a non-reentrant log sink, that constraint propagates.

Violating the render-thread-only rules is an assertion in debug builds and undefined behavior in release.

### 3.3 Multithreaded Recording Pattern

Worker threads each own a private `CommandBuffer`, each backed by a private arena (`XeAllocator`). They record independently — no locks, no atomics. The render thread collects the filled buffers and submits them as a batch; the batch array's index order *is* the execution order. There is no implicit reordering.

### 3.4 Deferred Resource Creation Pattern

A recommended client idiom, not part of the renderer itself: worker threads that need to create resources enqueue descriptors into a lock-free ring buffer; the render thread drains the ring at the top of `begin_frame()` before any command buffer references the new handles. This keeps the synchronization outside the middleware while giving worker code an apparent creation surface.

---

## 4. TierLevel

A five-valued `uint8_t` enum capturing the coarsest capability boundary across the supported hardware range:

- **Microcode** — N64 RDP/RSP. Combiner-equation rasterizer, no shaders, no hardware T&L.
- **TEV** — GX on GameCube/Wii. Hardware T&L, up to 16 TEV stages, no GLSL.
- **Legacy** — GL 1.x–2.x. Matrix stacks, per-vertex lighting, fixed-function texturing.
- **Modern** — GL 3.3+, GLES 3, D3D11, Metal. Programmable shaders, VBOs, FBOs.
- **Extended** — GL 4.x, Vulkan, Metal 3. Compute, geometry, tessellation.

Tier is the coarse branch the client uses when selecting pipeline-state payloads (§14). Sub-tier features (anisotropic filtering, NPOT textures, instancing) are surfaced as bool flags in `GPUCaps` rather than appearing as new tiers — tiers are slow-moving; feature flags change with every GPU driver revision.

---

## 5. BackendHint

A `uint8_t` enum listing every concrete backend the middleware knows about: `Auto`, the desktop set (GLLegacy, GL4, D3D11, Vulkan), mobile (GLES2, GLES3, Metal), consoles (N64_RDP, GCN_GX, PS3_GCM), and software fallbacks (SoftBuiltin, SoftMesa, Null). `Auto` defers backend selection to `BackendRegistry::best_available()`.

**Implementation approach:** the enum is densely packed (no gaps) so that `BackendRegistry` can use it directly as an array index. Adding a new backend means appending a new enumerant before `_Count` — no reordering, no renumbering of existing values.

**Rationale:** dense enums make the registry's storage a plain array instead of a map. The constant-time lookup matters because `is_available()` is called during startup probing and must not regress when the backend list grows.

---

## 6. RenderSurface

A POD tagged union carrying the platform's surface identity. Fields include a `SurfaceType` discriminator; opaque `void*` fields for `native_window`, `native_display`, `metal_layer`, and `console_context`; a console buffer index; surface dimensions; color and depth `PixelFormat` hints; MSAA sample count; sRGB, HDR, and swap-interval flags.

**Implementation approach:** one struct carries every platform's identifiers; each backend reads only the fields it cares about. On 64-bit desktop the struct is roughly 64 bytes.

**Rationale:** the alternative — a virtual `Surface` class hierarchy — would require an allocation and a vtable for a type that is read exactly once at device creation. Paying a few unused `void*` (8 bytes each on 64-bit) avoids the allocation entirely and keeps the surface description trivially copyable across the factory boundary. The surface struct is the *only* point at which OS-specific window handles cross into the middleware, so isolating it in a POD stops that knowledge from cascading into a type hierarchy.

---

## 7. SystemCaps

`SystemCaps` is the central capabilities record — a POD aggregate of four sub-structs.

### 7.1 CPUCaps

Fields: brand string, physical/logical core counts, cache-line size, L1/L2/L3 cache sizes, x86 SIMD flags (SSE2/SSE4/AVX/AVX2), ARM flags (NEON/SVE/is-Apple-Silicon), PowerPC flags (AltiVec/is-PowerPC), `is_bigendian` (critical for texture byte-swapping on N64/GC/Wii/PS3 PPU), `is_64bit`, `is_arm`, and auxiliary-processor budgets — SPU count and local-store bytes on PS3; RSP DMEM/IMEM on N64.

Auxiliary-processor fields are zero on platforms without them, so client code can consult them uniformly without `#ifdef`.

### 7.2 GPUCaps

Fields: identification strings (renderer, vendor, api_version), `TierLevel tier`, GPU memory budgets (`vram_bytes`, `aram_bytes` for GC/Wii, `tmem_bytes` for N64, and the PS3 RSX partitions `rsx_local_bytes` / `rsx_main_bytes`), `is_uma`, resource limits (`max_texture_size`, `max_texture_units`, `max_uniform_bindings`, `max_vertex_attribs`, `max_msaa_samples`, `max_tev_stages`, `max_hw_lights`), feature flags (geometry shaders, tessellation, compute, instancing, VAO, FBO, anisotropic, tile memory, float textures, depth textures, NPOT support), texture-compression flags (S3TC, ETC2, ASTC, PVRTC, CMPR), `is_bigendian_gpu`, and `uses_gpu_command_list`.

**Corrections from rev 0.6:**

- The fields `rsx_local_bytes` and `rsx_main_bytes` have moved from `MemoryCaps` to `GPUCaps`. They describe GPU-addressable memory, not system RAM.
- The field formerly named `uses_display_list` has been renamed to `uses_gpu_command_list` to eliminate the collision with the GL Legacy driver-side `glNewList` optimization covered in §9.3. The two concepts are unrelated: `uses_gpu_command_list` marks backends whose primary submission model is a replayable command list (RDP, GX, RSX); the GL Legacy display-list optimization is a backend-private implementation detail.

### 7.3 MemoryCaps

Fields: `system_ram_bytes`, `available_ram_bytes`, `page_size_bytes`, `has_mmu` (false on bare-metal N64), `has_large_pages`, `is_low_memory_device`, and `display_list_pool_bytes` for GX/rdpq display-list budgeting.

### 7.4 PlatformCaps

OS identification, mobility/tablet/console flags, `is_low_power_mode`, `has_os` (false on bare N64), `has_filesystem`, and `display_scale`.

### 7.5 Two-Phase Query

`SystemCaps::query_host(const XeServices&)` fills `CPUCaps`, `MemoryCaps`, `PlatformCaps` — everything available before GL/D3D/GCM context creation. `SystemCaps::query_gpu(const XeServices&)` is called by the device factory after context creation to populate `GPUCaps`. Two phases are mandatory because GPU identification requires a live context, but backend selection requires knowledge of system memory *before* the context exists.

**Implementation approach:** POD struct, default-constructed to all zero, no heap. All query functions are free or static — no dependency on a global or singleton.

**Rationale:** every field being directly readable avoids accessor boilerplate; `SystemCaps` is the most-referenced struct in the middleware, and keeping it a POD makes reading it a cache-line hit. The two-phase split is a hard requirement: any design that merged them would either force the client to create and destroy a throwaway context (wasted work) or require backend selection without capability information (defeats the registry's purpose).

---

## 8. XeServices — Facility Injection Hub

`XeServices` aggregates four sinks the middleware needs from the client — allocator, log sink, profiler hook, assert handler — plus an optional allocation tracker carried by pointer.

### 8.1 Sink Shapes

- **`XeLogSink`** — `write(user, level, tag, msg, file, line)` function pointer plus `user_data` plus a `min_level` filter. Log levels: Trace, Debug, Info, Warn, Error, Fatal.
- **`XeProfilerHook`** — `begin_cpu_scope`, `end_cpu_scope`, `mark`, `gpu_timestamp`, `counter`. Every hook takes a `void* user`. No hook is allowed to allocate in the steady state.
- **`XeAllocator`** — `alloc(user, size, align)`, `free(user, ptr, size, align)`, `realloc(user, ptr, old_size, new_size, align)`. The middleware always passes matching size/align to free, enabling sized-free allocators (musl, mimalloc) to skip the bookkeeping lookup.
- **`XeAllocTracker`** (optional) — `on_alloc`, `on_free` with tag/file/line for leak detection.
- **`XeAssertHandler`** — `on_assert(user, expr, msg, file, line) → bool`; return false to abort.

### 8.2 Implementation Approach

Each sink is a POD of C-style function pointers plus `void* user_data`; there is no `std::function`, no virtual dispatch, no RTTI. `XeServices::make_default()` returns a value wired to platform-natural primitives: `malloc`/`free` on desktop, `memalign`/`free` on N64, `abort()` for unhandled asserts. Clients override individual function pointers — never inherit.

Internal use is gated through macros (`XE_LOG`, `XE_ASSERT`, `XE_TRACE_SCOPE`, `XE_ALLOC`, `XE_FREE`) that compile away to nothing when `XE_DIAGNOSTICS_OFF` is defined — essential for N64 release builds where every kilobyte of `.text` counts.

### 8.3 Rationale

Function-pointer injection has three important properties over virtual calls:

1. **ABI resilience.** A new field may be appended to a sink struct without breaking existing backends; the layout is not tied to a C++ type hierarchy's vtable.
2. **No hidden allocation.** A polymorphic sink would need storage for its derived-class state; a function-pointer sink carries its state in the `user_data` pointer that the client controls.
3. **Provable `noexcept`.** The sink boundary is C-ABI, so no destructor can run through it. The middleware can claim `noexcept` end-to-end.

---

## 9. BackendVTable and BackendContext

### 9.1 Factory

Each backend translation unit exports one C-ABI symbol:

`xe_backend_<name>_create(const RenderDeviceDesc* desc, BackendContext** out_ctx) → BackendVTable`

The function returns a filled `BackendVTable` value and writes a heap-allocated opaque `BackendContext*` into `out_ctx`. `BackendContext` is backend-private — the middleware never dereferences it. All backends are linked statically, gated by CMake options. There is no `dlopen`, no dynamic plugin loading.

**Rationale:** static linking keeps the binary flat for console targets (N64 has no dynamic loader), simplifies symbol stripping for release builds, and gives LTO the whole graph to inline. Dynamic plugin loading would also make handle generation counters (§10) harder to invalidate cleanly on backend reload.

### 9.2 BackendVTable Shape

A C struct of function pointers, grouped into sections:

- **Lifecycle** — `shutdown`, `begin_frame`, `end_frame`, `present`.
- **Buffer management** — `buffer_create`, `buffer_update`, `buffer_destroy`.
- **Texture management** — `texture_create`, `texture_update`, `texture_destroy`, `rendertarget_create`, `rendertarget_destroy`.
- **Sampler management** — `sampler_create`, `sampler_destroy`.
- **Shader management** (Modern/Extended only) — `shader_create`, `shader_destroy`.
- **Pipeline management** — `pipeline_create`, `pipeline_destroy`.
- **Command submission** — `submit_batch(ctx, const CommandBufferBase* const* bufs, uint32_t count)`.
- **Synchronization** — `flush`, `finish`.
- **Memory query** — `query_memory`.
- **Native handle access** — `get_native_handle`, `query_extension`.
- **Debug** — `set_debug_label(ctx, XeHandleType, handle, label)`, `get_error_string`.

**Command recording does not appear in the vtable.** `bind_*`, `draw_*`, `set_*` are recorded into a `CommandBuffer` (see §12) and cross the dispatch boundary exactly once per submit, not once per call. This is the single most important performance property of the middleware.

**Unsupported-slot contract:** a backend that cannot implement a slot installs a stub that logs through `XeServices` (obtained via `BackendContext`) and returns `XE_INVALID_HANDLE` for handle-returning slots or `nullptr` for pointer-returning slots. The application gates feature use on `SystemCaps`, not on slot presence — the stub is a defense-in-depth safety net.

**ABI stability rule:** new slots may be appended only. Existing slots may never be reordered or removed. This preserves the ability to mix backend object files from different middleware versions — a rare but valuable diagnostic scenario.

**Rationale for flat vtable vs. virtual class:** a vtable is a POD with known offsets. A virtual class forces a specific compiler's vtable layout, which varies across MSVC, GCC, and Clang and is not stable across C++ ABIs. Exporting a flat vtable from a C-ABI factory gives us platform-independent binary compatibility and clean LTO behavior.

**Rationale for excluding command recording:** draws and state binds happen tens of thousands of times per frame. Dispatching each through a function pointer would cost a branch-predictor miss and a cache-line load per call. Instead, `CommandBuffer` serializes commands into a backend-neutral byte stream (dynamic policy) or directly into the native command format (static policy); `submit_batch` crosses the dispatch boundary exactly once per frame.

### 9.3 GL Legacy — Display List Optimization for Sampler and Pipeline State

The GL Legacy backend exploits `glNewList` / `glCallList`. At `create_sampler()`, the backend compiles the sequence of `glTexParameter*` calls into a display list; at `bind_texture()`, one `glCallList(sampler_list)` replays all four to five parameter calls in a single driver entry. At `create_pipeline()`, the rasterizer/blend/depth-stencil/fixed-function state is similarly compiled; `bind_pipeline()` replays it with one list call.

**Correction from rev 0.6:** the earlier spec claimed `glBindTexture`, `glVertexPointer`, and `glDrawElements` "capture values at `glNewList` time, not at `glCallList` time." This is incorrect in detail:

- `glBindTexture` *is* listable under GL 1.1–2.1; it captures the *texture name* at compile time (not the texture contents).
- `glVertexPointer` is *not* compilable into a display list — it is client-side state in every GL profile that still exposes it.
- `glDrawElements` is listable, but compiling a draw into the list would capture the *element array pointer at compile time* — which is almost never what you want.

The corrected rule for this backend: *compile only the state you want permanently baked into the list. Never compile commands that reference client-side pointers (vertex arrays) or resources whose identity is decided per-bind (the currently bound texture).*

The middleware compiles into the sampler list only `glTexParameter*` calls — not `glBindTexture`. The pipeline list compiles only rasterizer/blend/depth state — not `glDrawElements`. Both compiled lists are replay-safe at any point, in any order, between texture binds.

**Rationale:** GL Legacy drivers carry heavy per-call overhead (call validation, argument marshalling, command-packet build). A display list collapses N calls into one and lets the driver pre-validate the list at compile time. On Mesa3D and older Windows GL drivers the speedup per bind is 3–5×. On GL4 core profile this optimization is unavailable (display lists were removed in 3.0 core) but also unnecessary — the driver's internal state cache is more effective than anything the client can construct.

#### GL Legacy Display List Usage Summary

| Object | Compiled at | Contains | Called at |
|---|---|---|---|
| Sampler state list | `create_sampler()` | `glTexParameter*` calls | `bind_texture()` after `glBindTexture` |
| Pipeline state list | `create_pipeline()` | rasterizer + blend + depth + fixed-function state | `bind_pipeline()` |
| Static geometry list | Optional, content-driven | `glBegin/glEnd` geometry | Static world draw calls only |

---

## 10. Handle Encoding

Every resource handle is a 32-bit unsigned integer with the following bit layout:

| Bits  | Field       | Width | Purpose                                 |
|-------|-------------|:-----:|-----------------------------------------|
| 31–28 | `type`      | 4     | `XeHandleType` — one of 15 categories + Invalid |
| 27–20 | `gen`       | 8     | Generation counter                      |
| 19–0  | `index`     | 20    | Slot index into the backend pool        |

`XE_INVALID_HANDLE == 0`. For this sentinel to be unambiguous, `XeHandleType::Invalid = 0` is reserved; `Buffer`, `Texture`, `Sampler`, `Shader`, `Pipeline`, `RenderTarget` start at 1. *Correction from rev 0.6, which placed `Buffer = 0`; that encoding made handle `0` simultaneously mean "invalid" and "buffer slot 0 generation 0".*

20 bits of index gives a pool capacity of ~1,048,576 resources per type — comfortably above any realistic streaming scenario. 8 bits of generation allow 256 recycles of the same slot before the counter wraps, at which point a stale handle could collide; in practice this is several orders of magnitude beyond per-frame churn and is already longer-lived than any reasonable caching strategy.

Helpers `xe_make_handle(type, gen, idx)` and `xe_decode_handle(h)` are provided in the middleware header. The backend's `resolve(handle)` path reads the type, checks it matches the expected category, indexes the slot, and checks the generation matches the slot's current generation — three AND/compare operations, no cache misses once the slot table is hot.

**Typed aliases** (`BufferHandle`, `TextureHandle`, …) are `using` aliases of `uint32_t`, not distinct classes. This preserves C-ABI compatibility and lets handles travel through the vtable without conversion. Mixing types at the call site is technically legal C++ but produces an assertion on resolve — the type tag will not match.

**Implementation approach:** each backend owns per-type pools. A pool is a `std::array`-sized (or hand-rolled fixed-capacity) array of slots plus a freelist of recycled indices. Allocating a handle pops from the freelist (or grows the high watermark), bumps the slot's generation, marks it live, and encodes (type, gen, idx) into the returned handle. Releasing marks the slot dead and pushes its index onto the freelist; the generation is *not* bumped on release — it is bumped on the next allocation, which both saves work and guarantees that released handles immediately become stale.

**Rationale for in-band encoding:** on console targets we cannot afford a parallel generation table — that would double the indirection count on every handle use. Packing gen+type into the handle itself keeps resolve a single AND + compare, cache-free if the slot fits in L1. The 20/8/4 split is an engineering compromise: larger indices reduce slot-recycling pressure (fewer generation bumps, longer stale-detection window) but eat into the generation bits. 20/8/4 hits the sweet spot for every identified use case in the engine.

---

## 11. Static vs Dynamic Dispatch — `RenderDeviceT<Policy>`

### 11.1 Policy Tags

Six compile-time tags, each an empty struct carrying no data:

- `BackendPolicy_Dynamic` — runtime vtable dispatch, default on desktop and mobile.
- `BackendPolicy_N64_RDP`, `BackendPolicy_GCN_GX`, `BackendPolicy_PS3_GCM` — console static dispatch.
- `BackendPolicy_GL4`, `BackendPolicy_SoftBuiltin` — optional static dispatch for dev/CI single-backend builds.

The policy tag is purely a compile-time marker. No backend-specific header is included through the policy header itself.

### 11.2 DispatchCore<Policy>

A thin class holding the vtable (by value) and the opaque `BackendContext*`. Every public method is an `inline` forwarder over the corresponding vtable slot.

**Implementation approach:** on the dynamic policy, each method compiles to one indirect call through the vtable. On any static policy, the specialized `DispatchCore<Tag>` replaces vtable indirection with a direct call to the backend's exported symbol — with LTO the dispatch shell disappears entirely, and the call site becomes a register-to-register hand-off into the backend.

**Rationale:** centralizing the `_vt.slot(_ctx, …)` pattern in one class keeps `RenderDeviceT` readable and prevents drift between the vtable layout and its public users. One forwarder is a handful of instructions; the compiler keeps `_vt` in registers across back-to-back calls (resource loading loops, batch binding), and the vtable itself is ~256 bytes — hot in L1 after the first draw.

### 11.3 RenderDeviceT<Policy>

Two specialization shapes matter: the dynamic one (desktop/mobile shipping builds) and the per-console static ones. The public method surface is *identical* across specializations — the client never sees the policy.

**Dynamic specialization (`BackendPolicy_Dynamic`):** every public method delegates to `_dispatch.method(args)`. The member layout is `DispatchCore + SystemCaps + XeServices + BackendHint`. `RenderDeviceT::create()` calls the backend factory, stores the returned `BackendContext*` in the dispatch core, and calls `SystemCaps::query_gpu()` to fill the GPU caps. `RenderDeviceT::destroy()` forwards to the vtable's `shutdown` slot and frees the device.

**Static specializations (`N64_RDP`, `GCN_GX`, `PS3_GCM`):** every method calls the backend's exported symbol directly. `BackendContext` is *embedded by value* — no heap allocation, no pointer chase. `get_native_handle()` is an array index, not a function pointer; the compiler reduces it to an address computation.

**`submit()` on the dynamic specialization** wraps `submit_batch` with `count = 1`, forwarding a pointer to the single command buffer. The wrapper is inlined.

**Rationale for embedding by value on consoles:** N64 and GameCube have no heap worth using — it is either absent (N64 with libdragon bare-metal) or a simple bump allocator with catastrophic fragmentation risk. Every runtime heap allocation we avoid is one less place the game can run out of memory. Embedding the context eliminates the allocation without changing the public API surface, which is the entire point of the policy mechanism.

**Rationale for template specialization over `#ifdef`:** templates give us one source tree with multiple backends compiled in. The alias (§11.5) makes that invisible to client code. Using `#ifdef` across the renderer would fork the source per platform and make cross-platform CI build verification impossible.

### 11.4 CommandBufferT<Policy>

The same policy parameterization applies to `CommandBuffer`. The public record surface is identical across specializations.

**Dynamic specialization:** commands serialize into a tagged byte stream — one opcode byte per command plus packed arguments. `submit()` walks the stream, decoding each tag into the appropriate vtable slot call.

**Static specializations:** commands serialize *directly into the native display-list format* — rdpq words on N64, GX FIFO on GameCube, libGCM command stream on PS3. `submit()` is a DMA kickoff with no decode.

`CommandBufferT` inherits from an opaque `CommandBufferBase` that exposes a single read accessor returning a `(const uint8_t*, uint32_t bytes)` pair. This is what the vtable's `submit_batch` sees — backends decode only that narrow surface without coupling to the policy-specific layout.

**Rationale for direct encoding on consoles:** on a 93 MHz R4300 (N64), a generic decode pass would cost thousands of cycles per draw — unaffordable when the frame budget is 16.6 ms at 60 fps. Going straight to rdpq words means every `draw_indexed` is a few writes to a DMA-ready buffer. On desktop, the decode pass is cheap relative to GPU submission latency, and the neutrality of the byte stream lets backends evolve without changing command encoding.

### 11.5 The Alias Convention — `xe_platform_config.hpp`

A single header, generated by CMake (or set manually by a `#define`), aliases `xe::RenderDevice` to the appropriate `RenderDeviceT<Tag>` and `xe::CommandBuffer` to the matching `CommandBufferT<Tag>`. Every downstream file references `xe::RenderDevice`; the policy never leaks.

**Rationale:** template parameters are a compile-time concern, not a public-API concern. Exposing them would force client code to either write templates or hard-code a backend. The alias layer makes client code uniform across platforms — the same file compiles on N64, PS3, and Linux with different behavior underneath.

---

## 12. Native Handle Access — Extension Headers

For clients that need to reach the underlying API object (for example, to plug in ImGui's GL backend, or to share a texture with a compute library), the middleware provides two vtable slots: `get_native_handle(XeHandleType, uint32_t) → void*` and `query_extension(uint32_t extension_id) → void*`.

Per-backend extension headers under `xe/native/` provide typed wrappers. Each exposes an `Ext` struct with a constexpr `kID` (a four-character code) and a table of backend-specific function pointers. The client includes the header only when it knows which backend is active — these headers are *never* pulled in by the middleware core, which keeps the engine layer free of backend symbols.

**Implementation approach:** the client calls `dev->query_extension(GL4DeviceExt::kID)`, receives a `void*` that the header's inline helper casts to `GL4DeviceExt*`, and from there invokes typed accessors. If the backend does not recognize the ID, it returns `nullptr`; helpers null-check and return 0/nullptr on mismatch. On a static backend policy, `get_native_handle` reduces to an array index — one address computation, no function call.

The existing headers cover GL4 (GLuint accessors, `make_current`/`release_current`, `insert_fence`), GL Legacy (GLuint accessors, compiled display-list names), GCN GX (`GXTexObj*`, raw DL buffer pointer), and PS3 GCM (`rsxBuffer*`, `gcmContextData*`, RSX offsets).

**Rationale:** forcing every advanced use case into the generic API would bloat the surface for everybody. Making native access opt-in keeps the default build clean, and the `kID` negotiation gives the backend full type control over what it exposes. No dynamic cast, no RTTI, no virtual dispatch.

---

## 13. SamplerDesc and SamplerHandle

`SamplerDesc` is a POD containing:

- wrap modes per axis (`WrapMode`: Repeat, ClampToEdge, MirroredRepeat, ClampToBorder)
- min and mag filter (`FilterMode`: Nearest, Linear, LinearMip, NearestMip, Anisotropic)
- anisotropy level
- LOD bias, min, and max
- sRGB flag
- comparison-enable and comparison function for shadow/depth samplers
- border color (used only with `ClampToBorder`)
- optional debug name

`SamplerHandle` is a 32-bit handle following §10's encoding.

### 13.1 Backend Implementations

| Backend | Implementation |
|---|---|
| GL Legacy | Compiled GL display list of `glTexParameter*` calls; replayed by `glCallList` at bind |
| GL4 | `glGenSamplers` / `glSamplerParameter*` — native sampler object |
| GLES2 | No sampler objects; per-texture `glTexParameter*` calls inline at bind |
| GLES3 | `glGenSamplers` — native sampler object (GLES3 core; *no extension required* — correction from rev 0.6 which cited `GL_OES_sampler_objects`, a GLES2 extension) |
| D3D11 | `ID3D11SamplerState` |
| Metal | `MTLSamplerState` |
| GCN_GX | No sampler object; state cached and applied at bind via `GX_InitTexObjFilterMode` etc. |
| N64_RDP | TMEM tile parameters embedded in the draw command; state applied at bind |
| PS3_GCM | `rsxTextureControl` + `rsxTextureFilter`; state stored, applied at bind |
| SoftBuiltin | Sampler parameters threaded into the tile inner loop |
| Null | No-op |

`CommandBuffer::bind_texture(TextureHandle, SamplerHandle, uint8_t unit)` takes both handles; samplers are reusable across textures.

### 13.2 Rationale

D3D11, Metal, and modern GL decoupled sampler state from texture state because the same texture is often used with different filtering in different passes — mip vs no-mip for UI vs scene, clamp vs repeat for atlas vs tiling, trilinear vs anisotropic based on distance LOD. Matching the modern model keeps the client code portable.

Backends that lack true sampler objects simply cache the state and apply it at bind, so the client pays nothing for the abstraction on those backends — the `SamplerHandle` resolves to a small state struct and is applied through whatever primitives that backend exposes.

---

## 14. PipelineDesc

A descriptor composed of tier-specific state plus common rendering state.

### 14.1 Common Fields

`tier` (discriminator), `vertex_layout`, `rasterizer`, `blend`, `depth_stencil`, `primitive`, `color_attachment_count`, `color_formats[8]`, `depth_format`, `msaa_samples`.

### 14.2 Tier-Specific Payload (one of four, selected by `tier`)

- **`MicrocodePipelineState`** (Microcode tier): RDP combiner equation, cycle type (1-cycle or 2-cycle), TMEM tile configuration, fill color for cycle-0 clears.
- **`TEVPipelineState`** (TEV tier): up to 16 TEV stage descriptors (color/alpha combiner equations, texture source, register routing), hardware light configuration (up to 8 lights), fog setup.
- **`FixedFunctionState`** (Legacy tier): matrix stack mode, enabled lights, material properties, fog, alpha test, texture environment (modulate/replace/decal), per-vertex lighting toggles.
- **`ProgrammableState`** (Modern/Extended tier): shader handles (vertex, fragment, optionally geometry/tessellation), push-constant layout, uniform buffer bindings, sampler bindings, per-attachment write mask.

### 14.3 Implementation Approach

A C-style `union` of the four payloads, discriminated by `tier`. The client populates exactly one branch; backends read only their tier's field. The `union` is wrapped in a named outer struct so the common fields precede it.

### 14.4 Rationale

The alternative — a single unified pipeline struct that merges every tier's state — would:

1. Balloon `sizeof(PipelineDesc)` with fields inapplicable to the target platform.
2. Confuse the client about which fields actually affect rendering on the active backend.
3. Force the asset compiler to emit every field, which fights the "platform-native binary" principle (§18).

The tier union makes the applicable state surface local to the tier the client is targeting, which matches how the client's shader/material system thinks about pipelines anyway. The discriminated union is decodable by the asset compiler and by the runtime without polymorphism or hidden allocation.

---

## 15. ShaderDesc

A descriptor carrying multiple source forms and multiple precompiled forms, plus stage, entry point, and debug name. Source forms cover GLSL, GLSL ES 1.00, HLSL, MSL, and Cg. Precompiled forms cover SPIR-V, DXBC, metallib, and Cg microcode — each as a `(const void* bytecode, uint32_t size)` pair.

**Implementation approach:** a POD with pointer+size pairs for every format. Unused pointers are `nullptr`. The backend walks its known formats in priority order (precompiled first, then source). The client owns the memory backing each pointer — the descriptor holds no allocations of its own.

**Rationale:** in a multi-platform engine the asset compiler emits every format for every shader source file; the runtime binary carries all of them and the backend picks one. Avoiding a polymorphic `ShaderDesc` hierarchy keeps the compiler output flat, mmappable, and free of version-sensitive vtable layouts.

---

## 16. BackendRegistry

A process-lifetime singleton holding a `BackendInfo` array — one entry per `BackendHint`. Each entry carries `hint`, `name` (human-readable), `short_name` (tokenized), `tier`, `available`, `hardware_accel`, and an `unavailable_reason` string (null when available).

Public methods: `probe(const XeServices&)`, `count()`, `get(uint32_t index)`, `find(BackendHint)`, `best_available()`, `is_available(BackendHint)`, `platform_candidates()`.

**Implementation approach:** a static array sized to `BackendHint::_Count`. Entries are zero-initialized. `probe` is idempotent and one-shot — repeat calls are no-ops. The probe routine for each backend typically creates a throwaway context, queries extensions, destroys the context, and records the result. No heap allocation; the registry's storage is `.bss`.

`best_available()` walks the array in descending tier order and returns the first hardware-accelerated, available entry. If none qualify, it falls back to `SoftBuiltin` and finally `Null`.

**Rationale:** backend probing is expensive (creating a test context alone can cost tens of milliseconds on desktop). Making the registry a static, thread-unsafe, one-shot matches that lifecycle exactly — the client cannot accidentally re-probe mid-frame.

---

## 17. RenderDeviceDesc

Carries: `surface` (§6), `backend_hint`, `services` (§8), resource pool upper bounds (`max_buffers`, `max_textures`, `max_samplers`, `max_pipelines`, `max_shaders`), `enable_debug_layer` flag, `soft_rast` (SoftRastDesc), and `gcm_fifo_kb` (PS3 GCM command buffer ring size, default 512 KB).

The pool upper bounds are respected by the backend: a `BufferHandle` pool is sized to `max_buffers` at creation. This is critical on consoles where a growing `std::vector` is unacceptable; a fixed-size pool means bounded memory, and the capacity can be computed offline from the asset compiler's peak-use report.

**`SoftRastDesc`** (not covered in rev 0.6; defined here): tile width/height in pixels, scanline batch size, per-pixel shader function pointer table, and a thread-affinity policy (single-threaded, coarse-tile-per-thread, or deferred — though the `XeJob` integration that powers multi-tile dispatch is deferred to 0.8). `SoftRastDesc::make_default()` returns sensible defaults (16×16 tiles, default shader table).

**Lifetime:** `RenderDeviceDesc` is consumed by the factory call; the middleware copies out the fields it needs. The embedded `XeServices` sinks are *referenced* — the client must keep the underlying `user_data` alive for the lifetime of the device.

---

## 18. Asset Pipeline

### 18.1 Architecture

Three stages: import (any source format → compiler IR), process (mesh optimization, texture compression, platform tiling, LOD, mip chain, animation baking, material baking, vertex quantization, bounds computation), export (IR → platform-native binary). The runtime never sees a source format.

Runtime binaries are `.xemesh`, `.xetex`, `.xeanim`, `.xepkg`. The runtime loader `mmap`s (or `fread`s) the file, casts the start to the header struct, fills the appropriate descriptor with pointers into the mapped region, and calls `create_*`. The create path on every backend is a DMA transfer — no decoder, no intermediate copy.

**Rationale:** runtime load cost is dominated by format conversion, not by I/O. Moving conversion offline collapses loading to a DMA copy. On N64 the difference is a 10-second load versus a 200 ms load; on desktop it is an invisible startup versus a visible one.

### 18.2 Compiler CLI

`xe-asset-compiler [options] <input> --target <platform> --output <path>`

Targets: `n64`, `gamecube`, `wii`, `ps3`, `desktop-gl4`, `desktop-d3d11`, `ios-metal`, `android`.

Options: `--lod-levels`, `--strip` (convert to triangle strips — benefits N64 and GC), `--quantize-pos` (`none` / `i16` / `i10`), `--anim-rate`, `--no-compress` (for debugging), `--verbose`.

### 18.3 `.xetex` — Texture Binary Format

A directly-mappable flat structure. Header (`XeTexHeader`): magic `"XETX"`, version, platform id, texture type, pixel format *already in platform-native encoding*, dimensions (width/height/depth/array count/mip count), sampler properties, data offset, data size. Followed by `MipDescriptor[mip_count]` (offset, size, width, height per mip). Followed by the pixel-data region, already tiled/compressed/mipped for the target GPU.

Runtime load consists of one cast, one descriptor fill from the header, one `create_texture` call, and one `create_sampler` call. The `initial_data` pointer points directly into the mapped file.

### 18.4 `.xetexprop` — Texture Properties Sidecar

A TOML sidecar checked in alongside the source image. Keys: `usage` (albedo/normal/roughness_metallic/emissive/ui/cubemap_face/heightmap), `srgb`, `wrap_u`/`wrap_v`, `filter_min`/`filter_mag`, `aniso_level`, `mip_policy`, `mip_filter`, `max_size`, `premul_alpha`, `pot_enforce`. Plus per-family compression overrides under `[compress.<family>]` tables.

**Correction from rev 0.6:** the compression buckets are *platform families*, not individual compiler targets. Families are `desktop`, `mobile`, `n64`, `gamecube`, `wii`, `ps3`. The compiler maps its `--target` to a family (e.g., `--target desktop-gl4` and `--target desktop-d3d11` both consult `[compress.desktop]`); a target may override the family default via `[compress.<target>]` if both tables exist. Missing sidecars trigger filename-convention inference (`_n` → normal, `_rm` → roughness/metallic, `_e` → emissive) and a warning.

### 18.5 `.xemesh` — Mesh Package Format

A chunked package: `XeMeshHeader` (magic `"XEMS"`, version, platform id, counts for meshes/materials/textures/skins/animations, chunk table offset), then the chunk table `(offset, size, type, name_hash)`, then chunks:

- **MESH** — `VertexLayout` + `BufferDesc` pairs for vertex and index buffers.
- **MATL** — baked `PipelineDesc` per material variant, tier-correct for the target platform.
- **TEXR** — `TextureDesc` + `SamplerDesc` per texture, with `initial_data` pointing into TEXD.
- **TEXD** — raw pixel data (tiled, compressed).
- **SKIN** — joint hierarchy and inverse bind matrices.
- **ANIM** — baked, quantized animation tracks (also externalizable to `.xeanim`).
- **LODS** — LOD mesh descriptors and distance thresholds.
- **META** — names, AABBs, LOD ranges, node hierarchy.

### 18.6 glTF → Runtime Concept Mapping

| glTF concept | Resolved at | Runtime type |
|---|---|---|
| `bufferView` (ARRAY_BUFFER) | Compiler | `BufferHandle` (vertex) |
| `bufferView` (ELEMENT_ARRAY_BUFFER) | Compiler | `BufferHandle` (index) |
| `accessor` | Compiler | `VertexAttrib` inside `VertexLayout` |
| `image` + `sampler` | Compiler (decode + compress + tile) | `TextureHandle` + `SamplerHandle` |
| `material` (PBR) | Compiler (tier-appropriate bake) | `PipelineHandle` |
| `mesh.primitive` | Compile-time + runtime | Draw call in `CommandBuffer` |
| Node TRS | Runtime | Push constants or uniform buffer |
| Skin + joints | Compiler (baked, quantized) | `BufferHandle` (joint palette) |
| Animation channels | Compiler (resampled + compressed) | `.xeanim` |
| Sparse accessors | Compiler (materialized dense) | Dense `BufferHandle` |

Sparse accessors are fully materialized at compile time — the one glTF concept that would otherwise require special runtime handling is eliminated.

### 18.7 CI Integration

The asset compiler runs in CI on every content-author commit. Compiled outputs are stored in the package registry alongside code build artifacts. Runtime binaries never touch `.glb`, `.fbx`, or `.png`.

---

## 19. Platform / Backend Availability Matrix

| Platform | GLLeg | GL4 | GLES2 | GLES3 | D3D11 | Metal | Vulkan | N64_RDP | GCN_GX | PS3_GCM | SoftBuiltin | SoftMesa | Null |
|---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Win 98/ME** | ✓ | – | – | – | – | – | – | – | – | – | ✓ | △ | ✓ |
| **Win XP/Vista** | ✓ | △ | – | – | – | – | – | – | – | – | ✓ | △ | ✓ |
| **Win 7+** | ✓ | ✓ | – | – | ✓ | – | – | – | – | – | ✓ | ✓ | ✓ |
| **Win 10/11** | ✓ | ✓ | – | – | ✓ | – | ✓ | – | – | – | ✓ | ✓ | ✓ |
| **Linux** | ✓ | ✓ | – | – | – | – | ✓ | – | – | – | ✓ | ✓ | ✓ |
| **macOS ≤14** | ✓¹ | ✓¹ | – | – | – | ✓ | – | – | – | – | ✓ | ✓ | ✓ |
| **macOS 15+** | – | – | ✓² | ✓² | – | ✓ | – | – | – | – | ✓ | ✓ | ✓ |
| **Android 4.x** | – | – | ✓ | – | – | – | – | – | – | – | ✓ | – | ✓ |
| **Android 7+** | – | – | ✓ | ✓ | – | – | ✓ | – | – | – | ✓ | △ | ✓ |
| **iOS 8–17** | – | – | ✓³ | ✓³ | – | ✓ | – | – | – | – | ✓ | – | ✓ |
| **Nintendo 64** | – | – | – | – | – | – | – | ✓ | – | – | ✓ | – | ✓ |
| **GameCube** | – | – | – | – | – | – | – | – | ✓ | – | ✓ | – | ✓ |
| **Wii** | – | – | – | – | – | – | – | – | ✓ | – | ✓ | – | ✓ |
| **PS3** | – | – | – | – | – | – | – | – | – | ✓ | ✓ | – | ✓ |

¹ GL deprecated on macOS 10.14; functional through macOS 13.
² Via ANGLE (GL ES over Metal).
³ GLES deprecated iOS 12; Metal is the primary path.
△ Conditional on external library availability (e.g., Mesa3D binaries on Win 9x; SwANGLE on Android 7+).

---

## 20. Additional Console Target Notes

### 20.1 Sega Dreamcast

**SDK:** KallistiOS + `sh-elf-gcc`. **Tier:** `Legacy`. GLdc (MIT) provides GL 1.2 over PVR — the existing `GLLegacy` Glaze-backed backend could run on Dreamcast via GLdc with minimal modifications. Future `BackendHint::Dreamcast_PVR` enumerant.

### 20.2 Original Xbox

**SDK:** nxdk (MIT) + LLVM/Clang. **Tier:** `Legacy` / `Modern` boundary (NV2A supports VS 1.1 / PS 1.4). Future `BackendHint::Xbox_D3D8`. Good candidate for a `LegacyShader` sub-tier revision.

Both are deferred — neither appears in the §19 availability matrix yet.

---

## 21. Render Graph — Why It Is Out of Scope

The render graph belongs in the engine layer *above* this middleware, not inside it. This middleware's contract is: *given resources and commands, get pixels onto a surface.* A render graph's contract is: *given a description of the entire frame, determine the optimal execution and resource plan.*

- Console backends (N64, GC/Wii) have no concept of pass dependencies, transient resource aliasing, or barrier insertion. Carrying render graph machinery into these backends would violate the portability goal.
- A render graph's output is a set of ordered `CommandBuffer`s submitted to a `RenderDevice`. That is precisely the API this layer already exposes — the render graph is a *client* of this layer, not a participant.
- `PassDesc` (color/depth targets, clear values, store actions) is already expressive enough for a render graph to drive without requiring any changes to this spec.

---

## 22. Class Relationship Diagram

```
 ┌─────────────────────────────────────────────────────────────────────────┐
 │                         Application Layer                               │
 │              (+ XeJob / XeThread — separate library)                   │
 └──┬──────────────┬──────────────┬───────────────┬───────────┬────────────┘
    │              │              │               │           │
    ▼              ▼              ▼               ▼           ▼
┌──────────┐ ┌──────────┐ ┌────────────┐ ┌───────────┐ ┌──────────┐
│XeServices│ │Backend   │ │SystemCaps  │ │CommandBuf │ │CommandBuf│
│Allocator │ │Registry  │ │CPUCaps     │ │ (thread A)│ │(thread B)│
│LogSink   │ │probe()   │ │GPUCaps     │ │ XeAlloc   │ │XeAlloc   │
│Profiler  │ │best_avail│ │MemoryCaps  │ └─────┬─────┘ └────┬─────┘
│Assert    │ └────┬─────┘ │PlatformCaps│       │             │
└────┬─────┘      │       └────────────┘       └──────┬──────┘
     │            │                                   │ submit_batch (render thread)
     │            ▼                                   ▼
     │  ┌──────────────────────────────────────────────────────────┐
     └─▶│        RenderDeviceT<Policy>                             │
        │                                                          │
        │  Policy=Dynamic:   DispatchCore → BackendVTable          │
        │  Policy=N64_RDP:   Direct calls, BackendContext by value │
        │  Policy=GCN_GX:    Direct calls, BackendContext by value │
        │  Policy=PS3_GCM:   Direct calls, BackendContext by value │
        │                                                          │
        │  get_native_handle()  → O(1) array index                 │
        │  query_extension()    → typed native struct              │
        │  backend_ctx()        → for extension header helpers     │
        └─────────────────┬────────────────────────────────────────┘
                          │  via BackendVTable (dynamic)
                          │  or direct call (static specialization)
     ┌────────────────────┼────────────────────────────────────────┐
     ▼                    ▼                                        ▼
┌──────────┐      ┌──────────────┐                       ┌──────────────┐
│ N64_RDP  │      │   GCN_GX     │                       │   PS3_GCM    │
│ libdragon│      │  libogc GX   │                       │  PSL1GHT     │
│ TL:Micro │      │  TL:TEV      │                       │  TL:Leg/Mod  │
│ rdpq DL  │      │  TEV stages  │                       │  Cg shaders  │
└──────────┘      └──────────────┘                       └──────────────┘
     ┌──────────┬──────────────┬──────────────┬──────────────────┐
     ▼          ▼              ▼              ▼                  ▼
  GLLegacy    GL4           D3D11/Metal   SoftBuiltin         SoftMesa
  (Glaze)   (Glaze)         (native)     (tile rast)         (Mesa3D)
  TL:Legacy TL:Modern       TL:Modern    TL:Legacy           TL:Extended
  DL-opt.   sampler obj     native smp   sw sampler          sw sampler

Extension headers (opt-in, xe/native/):
  xe_native_gl4.hpp        → GL4DeviceExt    (GLuint names, make_current, fence)
  xe_native_gl_legacy.hpp  → GL1DeviceExt    (GLuint names, display list names)
  xe_native_gcn_gx.hpp     → GXDeviceExt     (GXTexObj*, raw DL buffer)
  xe_native_ps3_gcm.hpp    → PS3GCMDeviceExt (rsxBuffer*, gcmContextData*)

Resource handles (uint32_t, type+gen+index encoding per §10):
  BufferHandle  TextureHandle  SamplerHandle  PipelineHandle  ShaderHandle
```

---

## 23. Initialization Flow

1. Build `XeServices` (`XeServices::make_default()` or custom).
2. Run `SystemCaps::query_host(svc)` — fills CPU, memory, platform caps. GPU caps remain zero.
3. Create an OS/console window and populate a `RenderSurface`.
4. Run `BackendRegistry::probe(svc)` and choose a `BackendHint` — either `best_available()` or a user override (typically to work around a driver bug).
5. Build a `RenderDeviceDesc` (surface, backend hint, services, pool sizes, GCM FIFO size), and call `RenderDevice::create(desc)`. The factory runs the chosen backend's create function, which internally calls `SystemCaps::query_gpu(svc)` before returning.
6. Load compiled assets: `mmap` the `.xemesh`/`.xetex` file, walk the chunk table, call `create_buffer` / `create_texture` / `create_sampler` / `create_pipeline` in a loop. No format decisions, no hidden allocations.
7. Per frame: `dev->begin_frame()`, record one or more `CommandBuffer`s (possibly from worker threads), `dev->submit(cmdbuf)` (or `submit_batch`), `dev->end_frame()`, `dev->present()`.
8. Shutdown: release resources in reverse order, then `RenderDevice::destroy(dev)`. The destroy path forwards to the backend's `shutdown` slot, which destroys the `BackendContext`.

---

## 24. Design Decisions

| Decision | Rationale |
|---|---|
| **`RenderDeviceT<Policy>` + `CommandBufferT<Policy>`** | Single template parameter; `using` alias in `xe_platform_config.hpp` prevents leakage upward; LTO collapses static builds to direct calls |
| **`BackendContext` embedded by value in static specializations** | Eliminates heap allocation and pointer chase on N64/GC/PS3 where every byte and cycle counts |
| **`get_native_handle` + `query_extension` in `BackendVTable`** | Typed, backend-specific escape hatch without polluting the API-neutral surface; extension headers are opt-in |
| **`SamplerHandle` as a distinct resource** | Matches GL4/D3D11/Metal's object model; allows sampler reuse across textures; on backends without sampler objects the state is cached and applied inline at bind — no API change for those backends |
| **GL Legacy display list for sampler + pipeline state** | One `glCallList` replaces many `glTexParameter*` calls and the rasterizer/blend/depth setup; compiled once at object creation; correct because both samplers and pipelines are immutable |
| **Command recording excluded from `BackendVTable`** | Per-draw function-pointer indirection is unaffordable; `CommandBuffer` crosses the dispatch boundary once per submit, not once per draw |
| **Asset compiler as a standalone CLI** | Heavy dependencies (meshoptimizer, Basis, libktx) never link into the runtime; compiler output size equals runtime memory cost with no hidden conversion buffers — essential for N64 memory budgeting |
| **`.xetexprop` sidecar** | Source images carry no semantic metadata; the sidecar provides usage, wrap/filter, and per-family compression overrides in a human-readable, version-controllable form |
| **`.xemesh` TEXD chunk separates descriptors from pixel data** | `initial_data` in `TextureDesc` points directly into the TEXD chunk; `create_texture()` is a DMA transfer with no intermediate copy |
| **Sparse accessor materialization at compile time** | Eliminates the one glTF concept that would require special runtime handling; the runtime loader never sees it |
| **In-band handle encoding (4/8/20 bits)** | One AND + compare resolves a handle with no parallel table lookup; generation catches stale handles across 256 recycles; 20 index bits cover 1M resources per type |
| **`XE_INVALID_HANDLE == 0` with `XeHandleType::Invalid = 0`** | Makes zero-initialized handle fields unambiguously invalid; matches the zero-default convention used elsewhere in the codebase |
| **`rsx_*_bytes` in `GPUCaps`** | GPU-addressable memory belongs with GPU caps; rev 0.6 placed it in `MemoryCaps` by mistake |
| **`uses_gpu_command_list` rename** | Disambiguates from GL Legacy `glNewList`; the two concepts are orthogonal |

---

## 25. Changes Since 0.6

- **§0 Executive Summary** added.
- **§2 Document Conventions** added — fills the §2 gap present in prior revisions.
- **§10 Handle Encoding** added as a dedicated section. `XeHandleType::Invalid = 0` now reserved; `Buffer`, `Texture`, etc. start at 1. `HANDLE_INDEX_MASK` formalized as `0x000FFFFF`.
- **§7 GPUCaps / MemoryCaps** — `rsx_local_bytes` / `rsx_main_bytes` moved from `MemoryCaps` to `GPUCaps`. `uses_display_list` renamed to `uses_gpu_command_list`.
- **§9.3 display list scope rules** — `glBindTexture` and `glVertexPointer` capture semantics corrected (`glBindTexture` is listable; `glVertexPointer` is not).
- **§9.2** — the absence of command-recording slots from `BackendVTable` is now stated explicitly with rationale.
- **§11.3** — the dynamic `submit()` wrapping `submit_batch` with count 1 is documented.
- **§13 Sampler backends** — GLES3 correction: sampler objects are core, not `GL_OES_sampler_objects`.
- **§14 PipelineDesc** — tier union described in prose; rev 0.6 referred to "Revision 0.3 Section 7" which was not available to readers.
- **§17 SoftRastDesc** — field set now described (rev 0.6 referenced but never defined).
- **§18.4 `.xetexprop`** — compression buckets clarified as platform *families*, not compiler targets; target override semantics documented.
- All code listings removed; struct and function shapes described as prose with implementation approach and technical rationale.

---

## 26. Out of Scope for This Iteration

- Compute pipelines and compute dispatch (Extended tier)
- Multi-GPU / multi-adapter enumeration
- Async resource streaming / transfer queues
- Ray tracing
- Explicit Vulkan-style memory heap sub-allocation
- Render graph / frame graph (rationale in §21)
- Material and scene graph systems (live above this layer)
- SPIRV-Cross integration for automatic shader cross-compilation
- `SoftBuiltin` multi-threaded tile dispatch (`XeJob` integration deferred to 0.8)
- SPU-accelerated tile dispatch on PS3 (handoff point defined in `CPUCaps.spu_count`; implementation deferred)
- Dreamcast `PVR` backend and Xbox OG `D3D8` backend (architecture noted in §20; deferred)
- `LegacyShader` sub-tier for VS 1.x / PS 1.x assembly shaders (Xbox OG NV2A)
- Pre-tiled `native_format` fast-path for GX CMPR uploads (deferred to 0.8)
- `.xepkg` multi-asset bundle format (structure noted; full spec deferred to 0.8)
- Static geometry display lists for world meshes (GL Legacy; architecture noted in §9.3; content-pipeline decision deferred)
