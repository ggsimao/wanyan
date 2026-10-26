#include "debug/gl_debug.h"
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>


namespace debug {
    namespace {
        const char* severityName(GLenum severity) {
            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:   return "error";
                case GL_DEBUG_SEVERITY_MEDIUM: return "warning";
                case GL_DEBUG_SEVERITY_LOW:    return "note";
                default:                       return "info";
            }
        }

        const char* sourceName(GLenum source) {
            switch (source) {
                case GL_DEBUG_SOURCE_API:             return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "window system";
                case GL_DEBUG_SOURCE_SHADER_COMPILER: return "shader compiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY:     return "third party";
                case GL_DEBUG_SOURCE_APPLICATION:     return "application";
                case GL_DEBUG_SOURCE_OTHER:           return "other";
                default:                              return "unknown";
            }
        }

        const char* typeName(GLenum type) {
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:               return "error";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "deprecated behavior";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "undefined behavior";
                case GL_DEBUG_TYPE_PORTABILITY:         return "portability";
                case GL_DEBUG_TYPE_PERFORMANCE:         return "performance";
                case GL_DEBUG_TYPE_MARKER:              return "marker";
                case GL_DEBUG_TYPE_PUSH_GROUP:          return "push group";
                case GL_DEBUG_TYPE_POP_GROUP:           return "pop group";
                case GL_DEBUG_TYPE_OTHER:               return "other";
                default:                                return "unknown";
            }
        }

        // Whether a debugger (gdb, lldb, an IDE) is attached to this process.
        // Linux only: the kernel lists the tracing process's id in
        // /proc/self/status, as "TracerPid:", and 0 means nobody is tracing.
        // Checked once, on first use; a debugger attached later isn't noticed.
        bool debuggerAttached() {
            static const bool attached = [] {
                std::ifstream status("/proc/self/status");
                const std::string prefix = "TracerPid:";
                std::string line;
                while (std::getline(status, line)) {
                    if (line.compare(0, prefix.size(), prefix) == 0) {
                        return std::stoi(line.substr(prefix.size())) != 0;
                    }
                }
                return false;  // no such file or line: not Linux, or not traced
            }();
            return attached;
        }

        void stopIfDebugging() {
            if (debuggerAttached()) {
                std::raise(SIGTRAP);
            }
        }

        void GLAPIENTRY printDebugMessage(GLenum source, GLenum type, GLuint /*id*/,
                                        GLenum severity, GLsizei /*length*/, const GLchar* message,
                                        const void* /*userParam*/) {

            // Some drivers report every buffer allocation as a notification;
            // those would bury the messages that matter.
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
                return;
            }
            std::cerr << "OpenGL " << severityName(severity) << " (type: " << typeName(type) << ", source: "
                      << sourceName(source) << "): " << message << '\n';

            if (type == GL_DEBUG_TYPE_ERROR)
                stopIfDebugging();
        }
    } // namespace

    void enableGlDebugOutput() {
        if (!GL_DEBUG_ENABLED)
            return;
        GLint flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // report from inside the failing call
            glDebugMessageCallback(printDebugMessage, nullptr);
        }
    }
} // namespace debug
