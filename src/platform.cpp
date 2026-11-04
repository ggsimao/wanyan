#include "platform.h"

#include "debug/gl_debug.h"

#include <GL/glew.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
    std::runtime_error sdlError(const std::string& what) {
        return std::runtime_error(std::format("{}: {}", what, SDL_GetError()));
    }

    // Whether an environment variable is switched on: set to anything other
    // than an empty string or "0". Read once at startup.
    bool envFlag(const char* name) {
        const char* value = std::getenv(name);
        if (value == nullptr) {
            return false;
        }
        const std::string_view text(value);
        return !text.empty() && text != "0";
    }
}  // namespace

Platform::Platform(const char* title, int width, int height) : _width(width), _height(height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        throw sdlError("Couldn't initialize SDL");
    }

    // From here on, a failure has to undo what already succeeded: a throwing
    // constructor never runs the destructor.
    try {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);  // the shaders use GLSL 4.30
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);  // outlines
        if (debug::GL_DEBUG_ENABLED)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

        _window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                                   height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!_window) {
            throw sdlError("Couldn't create the window");
        }

        _context = SDL_GL_CreateContext(_window);
        if (!_context) {
            throw sdlError("Couldn't create an OpenGL 4.3 core context");
        }

        glewExperimental = GL_TRUE;  // needed for core profiles
        const GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK) {
            throw std::runtime_error(std::format("Couldn't initialize GLEW: {}",
                reinterpret_cast<const char*>(glewGetErrorString(glewStatus))));
        }

        // glewInit queries extensions the old way, which a core profile
        // rejects with a harmless GL_INVALID_ENUM. Clear it so it isn't
        // mistaken for an error of ours.
        glGetError();

        const bool shaderExtension = static_cast<bool>(GLEW_ARB_shader_viewport_layer_array);
        GLint glMajor = 0, glMinor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
        glGetIntegerv(GL_MINOR_VERSION, &glMinor);
        const bool contextIs4_6 = glMajor > 4 || (glMajor == 4 && glMinor >= 6);
        const bool forcedUnlayering = envFlag("WANYAN_NO_LAYERED_SHADOWS");

        _gpuFeatures.layeredVertexOutput = (
            (shaderExtension ||
            contextIs4_6) &&
            !forcedUnlayering
        );
        _gpuFeatures.unlayeredWasForced = forcedUnlayering;

        if (!_gpuFeatures.layeredVertexOutput) {
            std::string reason;
            if (!(contextIs4_6 || shaderExtension)) reason = "Unsupported GL version with no extension.";
            if (forcedUnlayering) reason = "Unlayering was forced by environment variable WANYAN_NO_LAYERED_SHADOWS.";
            std::cerr << "Point shadow layering was disabled due to: " << reason << '\n';
        }

        SDL_GL_SetSwapInterval(1);            // vsync, as in Rust; failure isn't fatal
        SDL_SetRelativeMouseMode(SDL_TRUE);   // captures the mouse for the camera
        debug::enableGlDebugOutput();
    } catch (...) {
        release();
        throw;
    }
}

Platform::~Platform() {
    release();
}

void Platform::release() noexcept {
    if (_context) {
        SDL_GL_DeleteContext(_context);
        _context = nullptr;
    }
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    SDL_Quit();
}

void Platform::swapBuffers() {
    SDL_GL_SwapWindow(_window);
}

int Platform::width() const {
    return _width;
}

int Platform::height() const {
    return _height;
}

const GpuFeatures& Platform::gpuFeatures() const {
    return _gpuFeatures;
}
