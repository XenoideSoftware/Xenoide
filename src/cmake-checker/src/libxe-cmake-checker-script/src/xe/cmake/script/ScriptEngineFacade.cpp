#include "ScriptEngineFacade.h"

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

namespace xe::cmake::script {

    struct ScriptEngineFacade::Impl {
        chaiscript::ChaiScript engine;

        Impl() {
            engine.add(chaiscript::fun([]() { return 0; }), "noop");
        }
    };

    ScriptEngineFacade::ScriptEngineFacade() : impl_(std::make_unique<Impl>()) {
    }

    ScriptEngineFacade::~ScriptEngineFacade() = default;

    ScriptEngineFacade::ScriptEngineFacade(ScriptEngineFacade &&) noexcept = default;

    ScriptEngineFacade &ScriptEngineFacade::operator=(ScriptEngineFacade &&) noexcept = default;

    void ScriptEngineFacade::register_bindings(ChaiRegistrar registrar) {
        if (registrar) {
            registrar(static_cast<void *>(&impl_->engine));
        }
    }

    void ScriptEngineFacade::eval(std::string_view source) {
        impl_->engine.eval(std::string(source));
    }

    void *ScriptEngineFacade::raw_engine() const {
        return static_cast<void *>(&impl_->engine);
    }

    bool ScriptEngineFacade::has_function(std::string_view name) const {
        try {
            // Referencing a bare identifier returns the function value; undefined
            // symbols raise an eval error.
            impl_->engine.eval(std::string(name));
            return true;
        } catch (const chaiscript::exception::eval_error &) {
            return false;
        }
    }

} // namespace xe::cmake::script