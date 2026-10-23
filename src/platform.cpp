#include "platform.h"

#include "debug/gl_debug.h"

#include <GL/glew.h>

#include <iostream>  // the debug callback
#include <stdexcept>
#include <string>

namespace {
    std::runtime_error sdlError(const std::string& what) {
        return std::runtime_error(what + ": " + SDL_GetError());
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
        if (debug::DEBUG_ON)
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
            throw std::runtime_error(std::string("Couldn't initialize GLEW: ") +
                                     reinterpret_cast<const char*>(glewGetErrorString(glewStatus)));
        }
        // glewInit queries extensions the old way, which a core profile
        // rejects with a harmless GL_INVALID_ENUM. Clear it so it isn't
        // mistaken for an error of ours.
        glGetError();

        SDL_GL_SetSwapInterval(1);            // vsync, as in Rust; failure isn't fatal
        SDL_SetRelativeMouseMode(SDL_TRUE);   // captures the mouse for the camera
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
