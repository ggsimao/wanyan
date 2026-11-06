#include "drawables/mesh_buffers.h"

#include "data/buffer.h"
#include "data/vertex_array.h"
#include "scene/instance.h"
#include <map>
#include <tuple>
#include <vector>

namespace {
    constexpr GLuint MODEL_LOCATION = 3;   // mat4: 4 consecutive vec4 locations
    constexpr GLuint MODEL_COLUMNS = 4;
    constexpr GLuint NORMAL_LOCATION = MODEL_LOCATION + MODEL_COLUMNS;  // mat3: 3 consecutive vec3 locations
    constexpr GLuint NORMAL_COLUMNS = 3;
    constexpr auto STRIDE = static_cast<GLsizei>(sizeof(Instance));

    // Orders positions by x, then y, then z, so glm::vec3 can be a std::map key.
    struct PositionLess {
        bool operator()(const glm::vec3& a, const glm::vec3& b) const {
            return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
        }
    };

    std::vector<GLuint> combinedIndices(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
        std::vector<GLuint> combined;
        combined.reserve(indices.size() * 2);
        combined.insert(combined.end(), indices.begin(), indices.end());
        std::map<glm::vec3, GLuint, PositionLess> uniqueVertices;
        for (const auto i : indices) {
            auto pos = vertices[i].pos;
            auto index = uniqueVertices.try_emplace(pos, i).first->second;
            combined.emplace_back(index);
        }
        return combined;
    }

    class LayeredAttributes {
    public:
        LayeredAttributes(VertexArrayBinding& vao, GLsizei layers) : _vao(vao), _layers(layers) {
            if (layers > 1) {
                for (GLuint i = MODEL_LOCATION; i < MODEL_LOCATION + MODEL_COLUMNS + NORMAL_COLUMNS; ++i) {
                    vao.attribDivisor(i, layers);
                }
            }
        }
        ~LayeredAttributes() { // doesn't query like other guards, fine since it's only used here
            if (_layers > 1) {
                for (GLuint i = MODEL_LOCATION; i < MODEL_LOCATION + MODEL_COLUMNS + NORMAL_COLUMNS; ++i) {
                    _vao.attribDivisor(i, 1);
                }
            }
        }
        LayeredAttributes(const LayeredAttributes&) = delete;
        LayeredAttributes& operator=(const LayeredAttributes&) = delete;
        LayeredAttributes(LayeredAttributes&& other) = delete;
        LayeredAttributes& operator=(LayeredAttributes&& other) = delete;
    private:
        VertexArrayBinding& _vao;
        GLsizei _layers;
    };
}

MeshBuffers::MeshBuffers(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
                         std::initializer_list<VertexAttribute> attributes)
    : _indexCount(static_cast<GLsizei>(indices.size())), _vertexCount(static_cast<GLsizei>(vertices.size())) {
    VertexArrayBinding vao(_vao);
    {
        auto abo = vao.bindABO(_abo);
        abo.bufferData(vertices, GL_STATIC_DRAW);
        for (const VertexAttribute& attribute : attributes) {
            abo.attribPointer(attribute.location, 3, GL_FLOAT, sizeof(Vertex), attribute.offset);
        }
    }
    auto ebo = vao.bindEBO(_ebo);
    ebo.bufferData(combinedIndices(vertices, indices), GL_STATIC_DRAW);
}

void MeshBuffers::draw() const {
    VertexArrayBinding vao(_vao);
    vao.drawElements(_indexCount);
}

void MeshBuffers::drawVertices() const {
    VertexArrayBinding vao(_vao);
    vao.drawVertices(_vertexCount, GL_POINTS);
}

void MeshBuffers::drawPositions() const {
    VertexArrayBinding vao(_vao);
    vao.drawElements(_indexCount, GL_TRIANGLES, _indexCount);
}

void MeshBuffers::drawInstanced(GLsizei instances) const {
    VertexArrayBinding vao(_vao);
    vao.drawElementsInstanced(_indexCount, instances);
}

void MeshBuffers::drawVerticesInstanced(GLsizei instances) const {
    VertexArrayBinding vao(_vao);
    vao.drawVerticesInstanced(_vertexCount, GL_POINTS, instances);
}

void MeshBuffers::drawPositionsInstanced(GLsizei instances, GLsizei layers) const {
    VertexArrayBinding vao(_vao);
    LayeredAttributes la(vao, layers);
    vao.drawElementsInstanced(_indexCount, instances * layers, GL_TRIANGLES, _indexCount);
}

void MeshBuffers::setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const {
    VertexArrayBinding vao(_vao);
    auto ibo = vao.bindABO(instanceBuffer);
    for (GLuint column = 0; column < MODEL_COLUMNS; ++column) {
        ibo.attribPointer(MODEL_LOCATION + column, 4, GL_FLOAT, STRIDE,
                          Instance::modelOffset() + column * sizeof(glm::vec4));
        vao.attribDivisor(MODEL_LOCATION + column, 1);  // advance once per instance
    }
    for (GLuint column = 0; column < NORMAL_COLUMNS; ++column) {
        ibo.attribPointer(NORMAL_LOCATION + column, 3, GL_FLOAT, STRIDE,
                          Instance::normalOffset() + column * sizeof(glm::vec3));
        vao.attribDivisor(NORMAL_LOCATION + column, 1);
    }
}

GLsizei MeshBuffers::indexCount() const {
    return _indexCount;
}
