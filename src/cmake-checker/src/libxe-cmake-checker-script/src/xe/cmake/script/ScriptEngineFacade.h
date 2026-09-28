#ifndef XE_CMAKE_SCRIPT_SCRIPT_ENGINE_FACADE_H
#define XE_CMAKE_SCRIPT_SCRIPT_ENGINE_FACADE_H

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace xe::cmake::script {

    // Callback that receives the opaque ChaiScript engine pointer so bindings can
    // be registered without leaking ChaiScript types into the facade header.
    using ChaiRegistrar = std::function<void(void *)>;

    // Opaque Pimpl facade isolating ChaiScript 6.1.0 template instantiation and
    // header overhead. No ChaiScript header leaks outside this library.
    class ScriptEngineFacade {
    public:
        ScriptEngineFacade();
        ~ScriptEngineFacade();
        ScriptEngineFacade(const ScriptEngineFacade &) = delete;
        ScriptEngineFacade &operator=(const ScriptEngineFacade &) = delete;
        ScriptEngineFacade(ScriptEngineFacade &&) noexcept;
        ScriptEngineFacade &operator=(ScriptEngineFacade &&) noexcept;

        // Registers a binding callback executed against the underlying engine.
        void register_bindings(ChaiRegistrar registrar);

        // Executes a ChaiScript source snippet.
        void eval(std::string_view source);

        // Returns the opaque engine pointer (internal use only).
        void *raw_engine() const;

        // Returns whether a function with the given name is defined.
        bool has_function(std::string_view name) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace xe::cmake::script

#endif // XE_CMAKE_SCRIPT_SCRIPT_ENGINE_FACADE_H