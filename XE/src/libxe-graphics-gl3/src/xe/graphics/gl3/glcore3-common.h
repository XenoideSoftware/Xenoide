
#pragma once

#include <optional>
#include <glaze/gl.hpp>
#include <glaze/raii.hpp>

namespace xe {
    /**
     * @brief A single entry in a backend resource pool.
     *
     * Pairs the RAII holder for the underlying GL object with an 8-bit generation counter.
     * A slot is considered free iff its RAII holder evaluates to false (the GL object has been
     * reset()); no separate liveness flag is needed. The generation is bumped by acquireSlot
     * when the slot is reused, so any stale handle that still references this index will fail
     * the gen() == slot.gen check enforced by the backend.
     *
     * @tparam T the underlying GL resource type (gl::BufferId, gl::Texture, gl::Program, ...)
     */
    template <class T> struct Slot {
        //! RAII-owned GL object. Empty (operator bool -> false) when the slot is free for reuse.
        glaze::Unique<T> obj;

        //! Authoritative generation counter. Bumped by acquireSlot when this slot is reused.
        uint8_t gen = 0;
    };

    /**
     * @brief A pool slot for non-GL resources (e.g. VertexLayoutGL) that live as plain C++ values.
     *
     * Shares the contract used by Slot<T>: the `obj` field evaluates to false iff the slot is free,
     * and the 8-bit generation counter is bumped on reuse so stale handles fail the gen check. Uses
     * std::optional as the liveness wrapper because glaze::Unique is specific to GL handle types.
     *
     * @tparam T the plain (non-GL) resource type held by the pool
     */
    template <class T> struct OptSlot {
        //! Engaged when the slot is live; empty after destroy, awaiting reuse.
        std::optional<T> obj;

        //! Authoritative generation counter. Bumped by acquireSlot when this slot is reused.
        uint8_t gen = 0;
    };

    /**
     * @brief Validate a handle against an OptSlot pool and return a pointer to the held object, or nullptr.
     *
     * Performs the full three-step handle check (index in range, slot live, generation matches). Returns
     * a borrowed pointer into the pool so callers avoid copying the underlying resource (e.g. a
     * VertexLayoutGL's attribute vector). The pointer is valid until the pool is mutated by the next
     * acquireSlot / destroy call on this pool.
     *
     * @tparam ObjectT the plain resource type held by the pool
     * @tparam HandleT the typed handle kind (must expose index() and gen())
     * @param pool the OptSlot-backed pool to look up in
     * @param handle the handle to validate
     * @return const pointer to the held object on success; nullptr if any check fails
     */
    template <typename ObjectT, typename HandleT> const ObjectT *tryObjectExtract(const std::vector<OptSlot<ObjectT>> &pool, const HandleT &handle) {
        uint32_t const index = handle.index();
        if (index >= pool.size()) {
            return nullptr;
        }

        OptSlot<ObjectT> const &slot = pool[index];
        if (!slot.obj) {
            return nullptr;
        }
        if (slot.gen != handle.gen()) {
            return nullptr;
        }

        return &*slot.obj;
    }

    // 16-bit index field in the handle bit layout; caps every pool at 65536 entries.
    constexpr uint32_t kHandleIndexLimit = 0x10000u;
    // 8-bit gen field in the handle bit layout; rolls over every 256 reuses per slot.
    constexpr uint32_t kHandleGenMask = 0xFFu;

    /**
     * @brief True if the pool has room for one more acquire (either unused capacity or a freed slot).
     *
     * Called by the public factories before acquireSlot; converts the hard 16-bit index cap into
     * a soft tl::expected error (BackendErrorCode::HandlePoolExhausted). Scan cost is O(n) only on
     * the overflow path, which is unreachable under normal usage.
     *
     * Slot-shape agnostic: works for any slot type exposing an `obj` field contextually convertible
     * to bool (Slot<T> via glaze::Unique, OptSlot<T> via std::optional).
     *
     * @tparam SlotT the concrete slot struct (Slot<T> or OptSlot<T>)
     * @param pool the pool to inspect; not mutated
     */
    template <class SlotT> bool poolHasCapacity(const std::vector<SlotT> &pool) {
        if (pool.size() < kHandleIndexLimit)
            return true;
        for (auto const &s : pool) {
            if (!s.obj)
                return true;
        }
        return false;
    }

    /**
     * @brief Reuse an empty slot (or append a new one) and return (index, gen).
     *
     * Linear-scans pool looking for a slot whose holder is empty - those are free slots awaiting
     * reuse. On hit, moves obj into that slot and bumps its gen counter. On miss, appends a new
     * slot. Scan cost is O(n); resource creation is not on the frame hot path so this is
     * intentional.
     *
     * Callers must ensure poolHasCapacity(pool) is true before calling; the tail assert is a
     * defensive invariant, not a user-facing failure path.
     *
     * Slot-shape agnostic: accepts any slot type whose `obj` field is assignable from ObjT and
     * contextually convertible to bool (Slot<T> holding glaze::Unique<T>, or OptSlot<T> holding
     * std::optional<T>).
     *
     * @tparam SlotT the concrete slot struct (Slot<T> or OptSlot<T>)
     * @tparam ObjT type of the resource holder being installed (deduced)
     * @param pool the pool vector to acquire a slot in; grows if no free slot is available
     * @param obj resource holder to install at the acquired slot
     * @return {index, gen} pair ready to be passed to HandleT::make
     */
    template <class SlotT, class ObjT> std::pair<uint32_t, uint8_t> acquireSlot(std::vector<SlotT> &pool, ObjT &&obj) {
        for (uint32_t i = 0; i < pool.size(); ++i) {
            if (!pool[i].obj) {
                pool[i].obj = std::forward<ObjT>(obj);
                pool[i].gen = static_cast<uint8_t>((pool[i].gen + 1) & kHandleGenMask);
                return {i, pool[i].gen};
            }
        }
        assert(pool.size() < kHandleIndexLimit && "acquireSlot: pool exhausted - caller skipped poolHasCapacity");
        pool.push_back({std::forward<ObjT>(obj), 0});
        return {static_cast<uint32_t>(pool.size() - 1), 0};
    }

    struct ProgramGL;

    /**
     * @brief Resolve a ProgramGL from a shader pool slot, or nullptr on validation failure.
     *
     * Performs the full index-range + slot-alive + generation-match check. Returns a borrowed
     * pointer into the pool; the pointer is valid until the next mutation of the shader-program pool.
     */
    const ProgramGL *tryProgramExtract(const std::vector<OptSlot<ProgramGL>> &pool, ShaderHandle handle);
} // namespace xe
