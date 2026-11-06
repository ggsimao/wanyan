#pragma once

#include <SDL2/SDL.h>

struct GpuFeatures {
    bool layeredVertexOutput = false;
    bool unlayeredWasForced = false;
};

struct PlatformOptions {
    bool forcedUnlayering = false;
    bool noVSync = false;
    bool mouseCaptured = true;
};

// Owns SDL, the window and the OpenGL 4.3 core context, and loads the GL
// functions through GLEW. Construct it before any GL object, so that it's
// destroyed after all of them: every GL object must be deleted while the
// context still exists.
//
// In debug builds (NDEBUG undefined) it also requests a debug context and
// prints OpenGL's error and warning messages to stderr as they happen.
class Platform {
public:
    // Throws std::runtime_error, with SDL's or GLEW's message, on failure.
    Platform(const char* title, int width, int height, const PlatformOptions& options);
    ~Platform();
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;

    void swapBuffers();

    int width() const;
    int height() const;
    const GpuFeatures& gpuFeatures() const;

    friend class PlatformController;
private:
    void release() noexcept;
    void setMouseCapture(bool captured);

    SDL_Window* _window = nullptr;
    SDL_GLContext _context = nullptr;
    GpuFeatures _gpuFeatures;
    PlatformOptions _options;
    int _width;
    int _height;
};
