#pragma once

#include "components/behaviours/flashlight.h"
#include "components/behaviours/fly_camera.h"
#include "components/behaviours/pickable.h"
#include "components/behaviours/picker.h"
#include "components/behaviours/rock_drift.h"
#include "components/behaviours/spin.h"
#include "components/frame_context.h"

#include <variant>

using Behaviour = std::variant<Spin, RockDrift, FlyCamera, Flashlight, Pickable,
                               Picker>;  // one line to extend

inline BehaviourBase& base(Behaviour& behaviour) {
    return std::visit([](BehaviourBase& b) -> BehaviourBase& { return b; },
                      behaviour);
}
inline const BehaviourBase& base(const Behaviour& behaviour) {
    return std::visit(
        [](const BehaviourBase& b) -> const BehaviourBase& { return b; },
        behaviour);
}

inline void start(Behaviour& behaviour, const FrameContext& context) {
    if (!base(behaviour).active) {
        return;
    };
    std::visit(
        [&](auto& b) {
            if constexpr (requires { b.start(context); }) {
                b.start(context);
            }
        },
        behaviour);
}

inline void update(Behaviour& behaviour, const FrameContext& context) {
    if (!base(behaviour).active) {
        return;
    };
    std::visit(
        [&](auto& b) {
            if constexpr (requires { b.update(context); }) {
                b.update(context);
            }
        },
        behaviour);
}

inline void lateUpdate(Behaviour& behaviour, const FrameContext& context) {
    if (!base(behaviour).active) {
        return;
    };
    std::visit(
        [&](auto& b) {
            if constexpr (requires { b.lateUpdate(context); }) {
                b.lateUpdate(context);
            }
        },
        behaviour);
}

inline void onDestroy(Behaviour& behaviour, const FrameContext& context) {
    if (!base(behaviour).active) {
        return;
    };
    std::visit(
        [&](auto& b) {
            if constexpr (requires { b.onDestroy(context); }) {
                b.onDestroy(context);
            }
        },
        behaviour);
}
