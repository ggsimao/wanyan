#include <drawables/vertex.h>

Vertex::Vertex(const glm::vec3& pos) : pos(pos), normal(glm::vec3(0.0f)), texCoords(glm::vec3(0.0f)) {}

void Vertex::translate(const glm::vec3& offset) {
    pos += offset;
}

void Vertex::rotate(float angle, const glm::vec3& axis) {
    pos = glm::vec3(glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(pos, 0.0f));
}
