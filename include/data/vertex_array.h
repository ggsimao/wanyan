#pragma once

#include "buffer.h"
#include "scoped_binding.h"

#include <GL/glew.h>
#include <iterator>
#include <type_traits>

class VertexArray {
public:
    VertexArray();
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;
    ~VertexArray();
    GLuint id() const;
    void bind() const;
    static void clearBinding();
private:
    void release() noexcept;
    GLuint _id = 0;
};

class VertexArrayBinding : ScopedBinding<VertexArray> {
public:
    explicit VertexArrayBinding(const VertexArray& vao) : ScopedBinding(vao) {}
    class ArrayBufferBinding : ScopedBinding<ArrayBuffer> {
    public:
        void bufferData(const void* data, size_t size, GLenum usage);

        template <std::ranges::contiguous_range R>
            requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
        void bufferData(const R& data, GLenum usage) {
            using T = std::ranges::range_value_t<R>;
            bufferData(std::ranges::data(data), std::ranges::size(data) * sizeof(T), usage);
        }

        void attribPointer(GLuint index, GLint components, GLenum type, GLsizei stride,
                           std::size_t offset, GLboolean normalized = GL_FALSE);
        void attribDivisor(GLuint index, GLuint divisor);

    private:
        friend class VertexArrayBinding;
        explicit ArrayBufferBinding(const ArrayBuffer& abo) : ScopedBinding(abo) {}
    };

    class ElementArrayBufferBinding : ScopedBinding<ElementArrayBuffer> {
    public:
        void bufferData(const void* data, size_t size, GLenum usage);

        template <std::ranges::contiguous_range R>
            requires std::is_trivially_copyable_v<std::ranges::range_value_t<R>>
        void bufferData(const R& data, GLenum usage) {
            using T = std::ranges::range_value_t<R>;
            bufferData(std::ranges::data(data), std::ranges::size(data) * sizeof(T), usage);
        }


    private:
        friend class VertexArrayBinding;
        explicit ElementArrayBufferBinding(const ElementArrayBuffer& ebo) : ScopedBinding(ebo) {}
    };

    void drawVertices(GLsizei count, GLenum mode) const;
    void drawVerticesInstanced(GLsizei count, GLenum mode, GLsizei instances) const;
    void drawElements(GLsizei count, GLenum mode = GL_TRIANGLES) const;
    void drawElementsInstanced(GLsizei count, GLsizei instances, GLenum mode = GL_TRIANGLES) const;

    [[nodiscard]] ArrayBufferBinding bindABO(const ArrayBuffer& abo);
    [[nodiscard]] ElementArrayBufferBinding bindEBO(const ElementArrayBuffer& ebo);
};
