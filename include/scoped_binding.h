#pragma once

#include <cassert>
#include <cstddef>

template <typename T>
inline constexpr bool restoreBindingOnExit = true;

// Binds an object for the duration of a scope, and restores whatever was bound
// before when the scope ends.
//
// T must provide a `bind() const` member and a static `clearBinding()`,
// unless Bind and Clear are given explicitly (as FrameBuffer does, with one
// pair of functions for each of its two binding targets). Each combination of
// T, Bind and Clear is tracked separately.
//
// - The bound object must outlive the binding: the binding keeps a pointer to
//   it, and binds it again when an inner binding ends. Constructing from
//   nullptr binds the default object instead (e.g. the window's framebuffer).
// - Bindings of the same kind may nest; ending one restores the one before it.
//   Using an outer binding while an inner one is active trips an assert,
//   since its object is no longer the bound one.
// - A type opts out of restoring on exit by specializing restoreBindingOnExit
//   to false. The element array buffer does: its binding is part of the VAO's
//   state, so undoing it would detach the VAO's index buffer.
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
