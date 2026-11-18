#pragma once

#include "data/buffer.h"
#include "scene/instance.h"
#include <span>

// Uploads instances to an array buffer outside of any VAO. The VAO-bound
// ArrayBufferBinding is for attribute setup; this only replaces data.
class InstanceBufferBinding : ScopedBinding<ArrayBuffer> {
  public:
    explicit InstanceBufferBinding(const ArrayBuffer& buffer)
        : ScopedBinding(buffer) {}

    void upload(std::span<const Instance> instances) {
        assertActive();
        // Rewritten whenever instances change, often every frame.
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(instances.size_bytes()),
                     instances.data(), GL_DYNAMIC_DRAW);
    }
};
