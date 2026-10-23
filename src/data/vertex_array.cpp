#include "data/vertex_array.h"
#include <stdexcept>
#include <utility>

VertexArray::VertexArray() {
    glGenVertexArrays(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a VAO");
}

VertexArray::VertexArray(VertexArray&& other) noexcept : _id(std::exchange(other._id, 0)) {}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }

    return *this;
}

VertexArray::~VertexArray() {
    release();
}

void VertexArray::release() noexcept {
    if (_id != 0) {
        glDeleteVertexArrays(1, &_id);
        _id = 0;
    }
}

GLuint VertexArray::id() const {
    return _id;
}

void VertexArray::bind() const {
    glBindVertexArray(_id);
}

void VertexArray::clearBinding() {
    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VertexArrayBinding::drawVertices(GLsizei count, GLenum mode) const {
    assertActive();
    glDrawArrays(mode, 0, count);
}

void VertexArrayBinding::drawVerticesInstanced(GLsizei count, GLenum mode, GLsizei instances) const {
    assertActive();
    glDrawArraysInstanced(mode, 0, count, instances);
}

void VertexArrayBinding::drawElements(GLsizei count, GLenum mode) const {
    assertActive();
    glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
}

void VertexArrayBinding::drawElementsInstanced(GLsizei count, GLsizei instances,
                                               GLenum mode) const {
    assertActive();
    glDrawElementsInstanced(mode, count, GL_UNSIGNED_INT, nullptr, instances);
}

VertexArrayBinding::ArrayBufferBinding VertexArrayBinding::bindABO(const ArrayBuffer& abo) {
    assertActive();
    return VertexArrayBinding::ArrayBufferBinding(abo);
}

VertexArrayBinding::ElementArrayBufferBinding VertexArrayBinding::bindEBO(const ElementArrayBuffer& ebo) {
    assertActive();
    return VertexArrayBinding::ElementArrayBufferBinding(ebo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VertexArrayBinding::ArrayBufferBinding::bufferData(const void* data, size_t size, GLenum usage) {
    assertActive();
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, usage);
}

void VertexArrayBinding::ArrayBufferBinding::attribPointer(GLuint index, GLint components,
                                                           GLenum type, GLsizei stride,
                                                           std::size_t offset,
                                                           GLboolean normalized) {
    assertActive();
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, components, type, normalized, stride,
                          reinterpret_cast<const void*>(offset));
}

void VertexArrayBinding::ArrayBufferBinding::attribDivisor(GLuint index, GLuint divisor) {
    assertActive();
    glVertexAttribDivisor(index, divisor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VertexArrayBinding::ElementArrayBufferBinding::bufferData(const void* data, size_t size, GLenum usage) {
    assertActive();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, usage);
}
