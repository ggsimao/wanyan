#pragma once

#include <memory>
#include <variant>
#include <vector>
#include <SDL2/SDL_events.h>

struct SignalKeyPressed {
    SDL_Keysym code;
};
struct SignalKeyReleased {
    SDL_Keysym code;
};
// Mouse movement since the previous event, in pixels.
struct SignalMouseMoved {
    int dx;
    int dy;
};
struct SignalMouseScrolled {
    int delta;
};
struct SignalQuit {};
using SignalType =
    std::variant<SignalKeyPressed, SignalKeyReleased, SignalMouseMoved, SignalMouseScrolled, SignalQuit>;

class Slot {
public:
    virtual ~Slot() = default;
    virtual void onSignal(const SignalType& signal) = 0;
};

class SignalHandler {
public:
    SignalHandler();
    ~SignalHandler();
    void connect(const std::shared_ptr<Slot>& slot);
    void waitEvent();
private:
    void emit(const SignalType& signal_value);
    std::vector<std::weak_ptr<Slot>> _slots;
};

template<typename O>
class Controller {
public:
    virtual ~Controller() = default;
    virtual void processSignals(O& obj) = 0;
};
