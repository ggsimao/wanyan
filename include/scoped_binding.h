#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
inline constexpr bool restoreBindingOnExit = true;

template <typename T,
          void (T::*Bind)() const = &T::bind,
          void (*Clear)() = &T::clearBinding>
class ScopedBinding {
public:
    ScopedBinding(const ScopedBinding&) = delete;
    ScopedBinding& operator=(const ScopedBinding&) = delete;
    ScopedBinding(ScopedBinding&&) = delete;
    ScopedBinding& operator=(ScopedBinding&&) = delete;

protected:
    explicit ScopedBinding(const T& obj) : _obj(&obj), _outer(s_active) {
        s_active = this;
        apply();
    }
    // Binds the default object (e.g. the window's framebuffer).
    explicit ScopedBinding(std::nullptr_t) : _obj(nullptr), _outer(s_active) {
        s_active = this;
        apply();
    }
    ~ScopedBinding() {
        assert(s_active == this && "Bindings destroyed out of order");
        s_active = _outer;
        if constexpr (restoreBindingOnExit<T>) {
            if (_outer) _outer->apply();
            else        Clear();
        }
    }
    void assertActive() const {
        assert(s_active == this && "Using a binding that has been superseded");
    }

private:
    void apply() const {
        if (_obj) (_obj->*Bind)();
        else      Clear();
    }

    const T* _obj;
    const ScopedBinding* _outer;
    inline static const ScopedBinding* s_active = nullptr;
};
