#pragma once

#include "scoped_binding.h"

#include <GL/glew.h>

template <GLenum target>
class Buffer {
public:
    Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;
    ~Buffer();
    GLuint id() const;
    void bind() const;
    static void clearBinding();
private:
    void release() noexcept;
    GLuint _id = 0;
};

using ArrayBuffer = Buffer<GL_ARRAY_BUFFER>;
using ElementArrayBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
using UniformBufferObject = Buffer<GL_UNIFORM_BUFFER>;

template <>
inline constexpr bool restoreBindingOnExit<ElementArrayBuffer> = false;
