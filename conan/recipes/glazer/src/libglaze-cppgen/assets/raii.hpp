#pragma once
/*
 * glaze — RAII smart-pointer wrappers for OpenGL resource types.
 *
 * This file is shipped as a static asset by the glaze Conan package; it is
 * NOT generated. The templates here are intentionally API-agnostic so a single
 * copy serves every supported OpenGL version (gl, gles1, gles2, glsc2, …).
 *
 * Design summary
 * --------------
 * The smart pointers follow std::unique_ptr / std::shared_ptr conventions:
 * one template parameter T naming the value type (typically gl::BufferId,
 * gl::dsa::Buffer, gl::handle::Buffer, …). The pointer stores T by value;
 * operator->() returns T*, so users call wrapper methods directly:
 *
 *     auto buf = glaze::makeUnique<gl::dsa::Buffer>();
 *     buf->data(GL_STATIC_DRAW, sizeof(verts), verts);   // → glNamedBufferData
 *
 * The bridge between the generic templates and any concrete type T is the
 * glaze::Traits<T> customization point. The generated per-API headers
 * (gl.hpp, gl_handle.hpp) specialize Traits<T> for every (handle × wrapper)
 * combination glaze knows how to create and destroy.
 *
 * Each specialization must provide:
 *
 *     static T    create();             // produce a fresh resource
 *     static void destroy(T& v);        // release the resource (called when valid)
 *     static bool valid(const T& v);    // is v holding a non-null resource?
 *
 * T must be default-constructible to a "null" state and move-constructible.
 * valid(default-constructed) must return false. The smart pointer never
 * calls destroy on an invalid value, mirroring std::unique_ptr's behaviour
 * when its stored pointer is null.
 *
 * Code targeting C++11 only — no <type_traits>, no SFINAE, no fold expressions.
 */

#ifndef __GLAZE_RAII_HPP__
#define __GLAZE_RAII_HPP__

#include <atomic>
#include <utility>

namespace glaze {

// ── Customization point ─────────────────────────────────────────────────────
/**
 * @brief Trait class describing how to create, destroy, and validate a value
 * type T managed by glaze::Unique / Shared / Weak. The primary template is
 * intentionally undefined: instantiating it for an unsupported T produces a
 * clear "incomplete type" diagnostic at the point of use.
 */
template<typename T>
struct Traits;

template<typename T> class Weak;

// ── Unique<T> ───────────────────────────────────────────────────────────────
/**
 * @brief Move-only owning wrapper for a value type T whose lifetime is managed
 * via glaze::Traits<T>. Inspired by std::unique_ptr, but specialized to types
 * that are themselves the resource (or a thin wrapper around one) rather than
 * raw pointers. Zero overhead beyond the storage of T itself.
 */
template<typename T>
class Unique {
public:
    Unique() noexcept : value_() {}
    explicit Unique(T v) noexcept : value_(std::move(v)) {}
    ~Unique() { reset(); }

    Unique(const Unique&) = delete;
    Unique& operator=(const Unique&) = delete;

    Unique(Unique&& other) noexcept : value_(std::move(other.value_)) {
        other.value_ = T();
    }
    Unique& operator=(Unique&& other) noexcept {
        if (this != &other) {
            reset();
            value_ = std::move(other.value_);
            other.value_ = T();
        }
        return *this;
    }

    /** @brief Pointer-like access to the underlying value (e.g. wrapper methods). */
    T*       operator->()       noexcept { return &value_; }
    const T* operator->() const noexcept { return &value_; }
    /** @brief Reference access to the underlying value. */
    T&       operator*()        noexcept { return value_; }
    const T& operator*()  const noexcept { return value_; }

    /** @brief Direct access to the stored value (e.g. to read its raw id). */
    T&       get()       noexcept { return value_; }
    const T& get() const noexcept { return value_; }

    /**
     * @brief Relinquish ownership without destroying. The returned value is
     * moved out; the Unique is left in a default (null) state.
     */
    T release() noexcept {
        T tmp = std::move(value_);
        value_ = T();
        return tmp;
    }

    /**
     * @brief Destroy the currently-held value (if valid) and adopt @p nv.
     * Defaults to a null value, so reset() with no argument leaves *this empty.
     */
    void reset(T nv = T()) noexcept {
        if (Traits<T>::valid(value_)) {
            Traits<T>::destroy(value_);
        }
        value_ = std::move(nv);
    }

    void swap(Unique& other) noexcept {
        using std::swap;
        swap(value_, other.value_);
    }

    /** @brief True iff the held value is currently valid. */
    explicit operator bool() const noexcept { return Traits<T>::valid(value_); }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator T&() noexcept { return value_; }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator const T&() const noexcept { return value_; }

private:
    T value_;
};

// ── Shared<T> ───────────────────────────────────────────────────────────────
/**
 * @brief Atomically ref-counted owning wrapper for a value type T. Inspired by
 * std::shared_ptr; uses an internally allocated control block. Traits<T>::destroy
 * is invoked exactly once, when the last Shared instance is released. The
 * control block itself is freed only after the last associated Weak observer
 * is also gone.
 */
template<typename T>
class Shared {
    struct Ctrl {
        T value;
        std::atomic<long> strong_rc;
        std::atomic<long> weak_rc;  // +1 collectively held by all strong owners
        Ctrl(T v) : value(std::move(v)), strong_rc(1), weak_rc(1) {}
    };

    /**
     * @brief Private constructor used by Weak::lock() — assumes the caller has
     * already incremented strong_rc on @p ctrl.
     */
    struct AdoptStrongTag {};
    Shared(Ctrl* ctrl, AdoptStrongTag) noexcept : ctrl_(ctrl) {}

public:
    Shared() noexcept : ctrl_(nullptr) {}
    /**
     * @brief Wrap @p v in a new Shared. If v is invalid (Traits<T>::valid(v)
     * returns false) the Shared is left empty and no control block is
     * allocated, mirroring std::shared_ptr's behaviour for null pointers.
     */
    explicit Shared(T v)
        : ctrl_(Traits<T>::valid(v) ? new Ctrl(std::move(v)) : nullptr) {}
    ~Shared() { release(); }

    Shared(const Shared& other) noexcept : ctrl_(other.ctrl_) {
        if (ctrl_) {
            ctrl_->strong_rc.fetch_add(1, std::memory_order_relaxed);
        }
    }
    Shared& operator=(const Shared& other) noexcept {
        if (this != &other) {
            release();
            ctrl_ = other.ctrl_;
            if (ctrl_) {
                ctrl_->strong_rc.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }
    Shared(Shared&& other) noexcept : ctrl_(other.ctrl_) { other.ctrl_ = nullptr; }
    Shared& operator=(Shared&& other) noexcept {
        if (this != &other) {
            release();
            ctrl_ = other.ctrl_;
            other.ctrl_ = nullptr;
        }
        return *this;
    }

    /** @brief Pointer-like access to the underlying value. UB if empty. */
    T*       operator->()       noexcept { return &ctrl_->value; }
    const T* operator->() const noexcept { return &ctrl_->value; }
    T&       operator*()        noexcept { return ctrl_->value; }
    const T& operator*()  const noexcept { return ctrl_->value; }

    /**
     * @brief Direct access to the stored value, or a default-constructed T
     * when this Shared is empty (matching the existing accessor contract).
     */
    T get() const noexcept { return ctrl_ ? ctrl_->value : T(); }

    long use_count() const noexcept {
        return ctrl_ ? ctrl_->strong_rc.load(std::memory_order_acquire) : 0;
    }

    void reset() noexcept {
        release();
        ctrl_ = nullptr;
    }

    void swap(Shared& other) noexcept {
        Ctrl* tmp = ctrl_;
        ctrl_ = other.ctrl_;
        other.ctrl_ = tmp;
    }

    explicit operator bool() const noexcept { return ctrl_ != nullptr; }
    bool operator==(const Shared& o) const noexcept { return ctrl_ == o.ctrl_; }
    bool operator!=(const Shared& o) const noexcept { return ctrl_ != o.ctrl_; }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator T&() noexcept { return ctrl_->value; }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator const T&() const noexcept { return ctrl_->value; }

private:
    friend class Weak<T>;

    void release() noexcept {
        if (!ctrl_) return;
        if (ctrl_->strong_rc.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            Traits<T>::destroy(ctrl_->value);
            if (ctrl_->weak_rc.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete ctrl_;
            }
        }
    }
    Ctrl* ctrl_;
};

// ── Weak<T> ─────────────────────────────────────────────────────────────────
/**
 * @brief Non-owning observer of a Shared<T>. Inspired by std::weak_ptr.
 * Holds a reference to the control block (so the block survives even after
 * the last Shared is destroyed) but does *not* keep the underlying resource
 * alive. Use lock() to attempt promotion back to a Shared; the result is
 * empty if the resource has already been released.
 */
template<typename T>
class Weak {
    using SharedT = Shared<T>;
    using Ctrl = typename SharedT::Ctrl;

public:
    Weak() noexcept : ctrl_(nullptr) {}
    Weak(const SharedT& s) noexcept : ctrl_(s.ctrl_) {
        if (ctrl_) {
            ctrl_->weak_rc.fetch_add(1, std::memory_order_relaxed);
        }
    }
    ~Weak() { release(); }

    Weak(const Weak& other) noexcept : ctrl_(other.ctrl_) {
        if (ctrl_) {
            ctrl_->weak_rc.fetch_add(1, std::memory_order_relaxed);
        }
    }
    Weak& operator=(const Weak& other) noexcept {
        if (this != &other) {
            release();
            ctrl_ = other.ctrl_;
            if (ctrl_) {
                ctrl_->weak_rc.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }
    Weak& operator=(const SharedT& s) noexcept {
        release();
        ctrl_ = s.ctrl_;
        if (ctrl_) {
            ctrl_->weak_rc.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }
    Weak(Weak&& other) noexcept : ctrl_(other.ctrl_) { other.ctrl_ = nullptr; }
    Weak& operator=(Weak&& other) noexcept {
        if (this != &other) {
            release();
            ctrl_ = other.ctrl_;
            other.ctrl_ = nullptr;
        }
        return *this;
    }

    long use_count() const noexcept {
        return ctrl_ ? ctrl_->strong_rc.load(std::memory_order_acquire) : 0;
    }
    bool expired() const noexcept { return use_count() == 0; }

    void reset() noexcept {
        release();
        ctrl_ = nullptr;
    }
    void swap(Weak& other) noexcept {
        Ctrl* tmp = ctrl_;
        ctrl_ = other.ctrl_;
        other.ctrl_ = tmp;
    }

    /**
     * @brief Attempt to promote to a Shared. Returns an empty Shared if expired.
     */
    SharedT lock() const noexcept {
        if (!ctrl_) return SharedT();
        long expected = ctrl_->strong_rc.load(std::memory_order_relaxed);
        while (expected != 0) {
            if (ctrl_->strong_rc.compare_exchange_weak(
                    expected, expected + 1,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
                return SharedT(ctrl_, typename SharedT::AdoptStrongTag{});
            }
        }
        return SharedT();
    }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator T&() noexcept { return ctrl_->value; }

    /** 
     * @brief Implicit conversion to the held type T
     */
    operator const T&() const noexcept { return ctrl_->value; }

private:
    void release() noexcept {
        if (ctrl_ && ctrl_->weak_rc.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete ctrl_;
        }
    }
    Ctrl* ctrl_;
};

// ── Factory helpers ─────────────────────────────────────────────────────────
/**
 * @brief Allocate a fresh resource of type T (via Traits<T>::create(args...))
 * and wrap it in a Unique<T>. Extra arguments are perfect-forwarded to the
 * trait factory, so types whose creator requires parameters (e.g. gl::Shader,
 * whose glCreateShader takes a GLenum type) are constructible via
 * `glaze::makeUnique<gl::Shader>(gl::ShaderType::eVertexShader)`.
 */
template<typename T, typename... Args>
inline Unique<T> makeUnique(Args&&... args) {
    return Unique<T>(Traits<T>::create(std::forward<Args>(args)...));
}

/**
 * @brief Allocate a fresh resource of type T (via Traits<T>::create(args...))
 * and wrap it in a Shared<T>. Arguments are forwarded the same way as
 * makeUnique.
 */
template<typename T, typename... Args>
inline Shared<T> makeShared(Args&&... args) {
    return Shared<T>(Traits<T>::create(std::forward<Args>(args)...));
}

} // namespace glaze

#endif
