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
}  // namespace

Geometry cubeGeometry(float side) {
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
                glm::vec3(static_cast<float>(column), static_cast<float>(row), 0.0f);
        }
    }
    return cube;
}

Geometry squareGeometry(float side) {
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
                glm::vec3(static_cast<float>(column), static_cast<float>(1 - row), 0.0f);
        }
    }
    return square;
}

Geometry quadGeometry(float side) {
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
            glm::vec3(static_cast<float>(column), static_cast<float>(1 - row), 0.0f);
    }
    return quad;
}
