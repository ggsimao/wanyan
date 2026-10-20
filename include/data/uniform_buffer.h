#pragma once

#include "data/buffer.h"
#include "scoped_binding.h"

#include <GL/glew.h>
#include <glm/mat4x4.hpp>

// The shaders' shared "Matrices" uniform block, std140 layout:
//
//     layout (std140) uniform Matrices {
//         mat4 model;       // offset   0
//         mat4 view;        // offset  64
//         mat4 projection;  // offset 128
//         mat4 normal;      // offset 192
//     };
//
// Allocated on construction; bindBase() connects it to the block's binding point.
class UniformBuffer {
public:
    explicit UniformBuffer(GLuint bindingPoint);

    GLuint id() const;
    GLuint bindingPoint() const;

    // Attaches the buffer to its binding point. This is persistent, indexed
    // state, not a scoped binding: it stays until something else is attached.
    void bindBase() const;

    void setModelMat(const glm::mat4& model);
    void setViewMat(const glm::mat4& view);
    void setProjectionMat(const glm::mat4& projection);
    void setNormalMat(const glm::mat4& normal);

private:
    void setMat(GLintptr offset, const glm::mat4& value);

    UniformBufferObject _buffer;
    GLuint _bindingPoint;
};

// Configuration guard for uniform buffers.
class UniformBufferBinding : ScopedBinding<UniformBufferObject> {
public:
    explicit UniformBufferBinding(const UniformBufferObject& buffer) : ScopedBinding(buffer) {}

    void allocate(GLsizeiptr size, GLenum usage);
    void subData(GLintptr offset, GLsizeiptr size, const void* data);
};
