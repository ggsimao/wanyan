#include "drawables/geometry.h"

#include <glm/geometric.hpp>

#include <cstddef>

namespace {
    // Gives the four vertices of one quad face (firstVertex..firstVertex+3) the
    // normal of its first triangle (indices firstIndex..firstIndex+2).
    void assignFaceNormal(Geometry& geometry, std::size_t firstIndex, std::size_t firstVertex) {
        const auto& v = geometry.vertices;
        const auto& i = geometry.indices;
        const glm::vec3& a = v[i[firstIndex]].pos;
        const glm::vec3& b = v[i[firstIndex + 1]].pos;
        const glm::vec3& c = v[i[firstIndex + 2]].pos;
        const glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        for (std::size_t corner = 0; corner < 4; ++corner) {
            geometry.vertices[firstVertex + corner].normal = normal;
        }
    }

    constexpr float DEGENERATE_EPSILON = 1e-8f;

    // Must run after normals are set.
    void assignTangents(Geometry& geometry) {
        const auto& vertices = geometry.vertices;
        const auto& indices = geometry.indices;
        auto i_len = indices.size();
        assert(i_len % 3 == 0);
        for (std::size_t tri = 0; tri < i_len; tri+=3) {
            const Vertex& v1 = vertices[indices[tri]];
            const Vertex& v2 = vertices[indices[tri + 1]];
            const Vertex& v3 = vertices[indices[tri + 2]];
            const glm::vec3 e1 = v2.pos - v1.pos;
            const glm::vec3 e2 = v3.pos - v1.pos;
            const glm::vec2 dUV1 = v2.texCoords - v1.texCoords;
            const glm::vec2 dUV2 = v3.texCoords - v1.texCoords;
            const float det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
            glm::vec4 tangent;
            if (glm::abs(det) < DEGENERATE_EPSILON) {
                tangent = fallbackTangent(v1.normal);
            } else {
                const float f = 1.0f / det;

                glm::vec3 t{
                    f * (dUV2.y * e1.x - dUV1.y * e2.x),
                    f * (dUV2.y * e1.y - dUV1.y * e2.y),
                    f * (dUV2.y * e1.z - dUV1.y * e2.z)
                };

                glm::vec3 b{
                    f * (-dUV2.x * e1.x + dUV1.x * e2.x),
                    f * (-dUV2.x * e1.y + dUV1.x * e2.y),
                    f * (-dUV2.x * e1.z + dUV1.x * e2.z)
                };

                tangent = packTangent(v1.normal, t, b);
            }

            geometry.vertices[indices[tri]].tangent = tangent;
            geometry.vertices[indices[tri + 1]].tangent = tangent;
            geometry.vertices[indices[tri + 2]].tangent = tangent;
        }
    }
}  // namespace

Geometry cubeGeometry(float side, glm::vec2 texture_scale) {
    const float h = side / 2.0f;
    // Four vertices per face, in the same order as the Rust version so
    // windings and texture coordinates match.
    Geometry cube{
        {
            // top
            Vertex({-h, h, -h}), Vertex({h, h, -h}), Vertex({-h, h, h}), Vertex({h, h, h}),
            // bottom
            Vertex({-h, -h, -h}), Vertex({h, -h, -h}), Vertex({-h, -h, h}), Vertex({h, -h, h}),
            // back
            Vertex({h, h, -h}), Vertex({-h, h, -h}), Vertex({h, -h, -h}), Vertex({-h, -h, -h}),
            // right
            Vertex({h, h, h}), Vertex({h, h, -h}), Vertex({h, -h, h}), Vertex({h, -h, -h}),
            // front
            Vertex({-h, h, h}), Vertex({h, h, h}), Vertex({-h, -h, h}), Vertex({h, -h, h}),
            // left
            Vertex({-h, h, -h}), Vertex({-h, h, h}), Vertex({-h, -h, -h}), Vertex({-h, -h, h}),
        },
        {
            0,  2,  1,  1,  2,  3,   // top
            4,  5,  6,  6,  5,  7,   // bottom
            8,  10, 9,  9,  10, 11,  // back
            12, 14, 13, 13, 14, 15,  // right
            16, 18, 17, 17, 18, 19,  // front
            20, 22, 21, 21, 22, 23,  // left
        },
    };

    for (std::size_t face = 0; face < 6; ++face) {
        assignFaceNormal(cube, face * 6, face * 4);
        for (std::size_t corner = 0; corner < 4; ++corner) {
            const std::size_t column = corner % 2;
            const std::size_t row = corner / 2;
            cube.vertices[face * 4 + corner].texCoords =
                glm::vec2(static_cast<float>(column), static_cast<float>(row)) * texture_scale;
        }
    }

    assignTangents(cube);

    return cube;
}

Geometry squareGeometry(float side, glm::vec2 texture_scale) {
    const float h = side / 2.0f;
    // The same four corners twice: once facing +Z, once facing -Z.
    Geometry square{
        {
            Vertex({-h, h, 0.0f}), Vertex({h, h, 0.0f}), Vertex({-h, -h, 0.0f}), Vertex({h, -h, 0.0f}),
            Vertex({-h, h, 0.0f}), Vertex({h, h, 0.0f}), Vertex({-h, -h, 0.0f}), Vertex({h, -h, 0.0f}),
        },
        {
            0, 2, 1, 1, 2, 3,  // front
            4, 5, 6, 5, 7, 6,  // back: reversed winding
        },
    };

    for (std::size_t face = 0; face < 2; ++face) {
        assignFaceNormal(square, face * 6, face * 4);
        for (std::size_t corner = 0; corner < 4; ++corner) {
            const std::size_t column = corner % 2;
            const std::size_t row = corner / 2;
            square.vertices[face * 4 + corner].texCoords =
                glm::vec2(static_cast<float>(column), static_cast<float>(1 - row)) * texture_scale;
        }
    }

    assignTangents(square);

    return square;
}

Geometry quadGeometry(float side, glm::vec2 texture_scale) {
    const float h = side / 2.0f;
    Geometry quad{
        {Vertex({-h, h, 0.0f}), Vertex({h, h, 0.0f}), Vertex({-h, -h, 0.0f}), Vertex({h, -h, 0.0f})},
        {0, 2, 1, 1, 2, 3},
    };

    assignFaceNormal(quad, 0, 0);
    for (std::size_t corner = 0; corner < 4; ++corner) {
        const std::size_t column = corner % 2;
        const std::size_t row = corner / 2;
        quad.vertices[corner].texCoords =
            glm::vec2(static_cast<float>(column), static_cast<float>(1 - row)) * texture_scale;
    }

    assignTangents(quad);

    return quad;
}
