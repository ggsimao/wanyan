#include <data/buffer.h>
#include <stdexcept>
#include <utility>

template <GLenum target>
Buffer<target>::Buffer() {
    glGenBuffers(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a buffer object");
}

template <GLenum target>
Buffer<target>::Buffer(Buffer&& other) noexcept : _id(std::exchange(other._id, 0)) {}

template <GLenum target>
Buffer<target>& Buffer<target>::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }

    return *this;
}

template <GLenum target>
Buffer<target>::~Buffer() {
    release();
}

template <GLenum target>
void Buffer<target>::release() noexcept {
    if (_id != 0) {
        glDeleteBuffers(1, &_id);
        _id = 0;
    }
}

template <GLenum target>
GLuint Buffer<target>::id() const {
    return _id;
}

template <GLenum target>
void Buffer<target>::bind() const {
    glBindBuffer(target, _id);
}

template <GLenum target>
void Buffer<target>::clearBinding() {
    glBindBuffer(target, 0);
}

template class Buffer<GL_ARRAY_BUFFER>;
template class Buffer<GL_ELEMENT_ARRAY_BUFFER>;
template class Buffer<GL_UNIFORM_BUFFER>;
