#include "controls/controls.h"

#include <SDL_events.h>
#include <algorithm>

SignalHandler::SignalHandler() = default;

SignalHandler::~SignalHandler() = default;

void SignalHandler::connect(const std::shared_ptr<Slot>& slot) {
    if (slot) {
        _slots.push_back(slot);
    }
}

void SignalHandler::waitEvent() {
    SDL_Event ev;
    auto mouseCaptured = SDL_GetRelativeMouseMode();
    while(SDL_PollEvent(&ev)) {
        if (ev.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            emit(SignalFocusLost{});
        }
        switch (ev.type) {
            case SDL_KEYDOWN:
                // Holding a key makes SDL repeat KEYDOWN; only the first
                // press counts, or toggles (F, E, M...) would flicker.
                if (!ev.key.repeat) {
                    emit(SignalKeyPressed{ev.key.keysym});
                }
                break;
            case SDL_KEYUP:
                emit(SignalKeyReleased{ev.key.keysym});
                break;
            case SDL_MOUSEMOTION:
                if (mouseCaptured) {
                    // Relative motion since the last event, not the cursor's
                    // position: with relative mouse mode the cursor doesn't move.
                    emit(SignalMouseMoved{ev.motion.xrel, ev.motion.yrel});
                }
                break;
            case SDL_MOUSEWHEEL:
                if (mouseCaptured) {
                    emit(SignalMouseScrolled{ev.wheel.y});
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (!SDL_GetRelativeMouseMode()) {
                    emit(SignalCaptureRequested{});
                }
                break;
            case SDL_QUIT:
                emit(SignalQuit{});
                break;
            default:
                break;
        }
    }
}

void SignalHandler::emit(const SignalType& signal_value) {
    auto emit_or_remove = [&signal_value](const std::weak_ptr<Slot>& slot) -> bool {
        auto lock = slot.lock();
        if (lock) {
            lock->onSignal(signal_value);
            return false;
        } else return true;
    };
    _slots.erase(std::remove_if(_slots.begin(), _slots.end(), emit_or_remove), _slots.end());
}
