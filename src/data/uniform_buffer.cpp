#include "data/uniform_buffer.h"
#include <glm/fwd.hpp>

namespace {
    constexpr GLsizeiptr MAT4_SIZE = sizeof(glm::mat4);  // 64 bytes, also the std140 stride

    constexpr GLintptr MODEL_OFFSET = 0 * MAT4_SIZE;
    constexpr GLintptr PROJ_VIEW_OFFSET = 1 * MAT4_SIZE;
    // constexpr GLintptr VIEW_OFFSET = 1 * MAT4_SIZE;
    // constexpr GLintptr PROJECTION_OFFSET = 2 * MAT4_SIZE;
    constexpr GLintptr NORMAL_OFFSET = 2 * MAT4_SIZE;
    constexpr GLsizeiptr MATRICES_SIZE = 3 * MAT4_SIZE;

    static_assert(sizeof(glm::mat4) == 16 * sizeof(float),
                "glm::mat4 must be tightly packed to match std140");
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UniformBuffer::UniformBuffer(GLuint bindingPoint) : _bindingPoint(bindingPoint) {
    UniformBufferBinding ubo(_buffer);
    // Rewritten several times per frame (once per object for the model matrix).
    ubo.allocate(MATRICES_SIZE, GL_DYNAMIC_DRAW);
}

GLuint UniformBuffer::id() const {
    return _buffer.id();
}

GLuint UniformBuffer::bindingPoint() const {
    return _bindingPoint;
}

void UniformBuffer::bindBase() const {
    glBindBufferBase(GL_UNIFORM_BUFFER, _bindingPoint, _buffer.id());
}

void UniformBuffer::setModelMat(const glm::mat4& model) {
    setMat(MODEL_OFFSET, model);
}

void UniformBuffer::setProjViewMat(const glm::mat4& proj_view) {
    setMat(PROJ_VIEW_OFFSET, proj_view);
}

void UniformBuffer::setNormalMat(const glm::mat4& normal) {
    setMat(NORMAL_OFFSET, normal);
}

void UniformBuffer::setMat(GLintptr offset, const glm::mat4& value) {
    UniformBufferBinding ubo(_buffer);
    ubo.subData(offset, MAT4_SIZE, &value[0][0]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UniformBufferBinding::allocate(GLsizeiptr size, GLenum usage) {
    assertActive();
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
}

void UniformBufferBinding::subData(GLintptr offset, GLsizeiptr size, const void* data) {
    assertActive();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
