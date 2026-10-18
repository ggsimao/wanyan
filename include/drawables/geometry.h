#pragma once

#include "drawables/vertex.h"

#include <GL/glew.h>

#include <vector>

// CPU-side shape data, ready to upload into MeshBuffers.
struct Geometry {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

// Centred on the origin. Flat-shaded: four vertices per face, each face with
// its own normal; texture coordinates span each face.
Geometry cubeGeometry(float side);

// In the XY plane, centred on the origin, with a front (+Z) and a back (-Z) face.
Geometry squareGeometry(float side);

// Like squareGeometry, but front face only. quadGeometry(2.0f) covers
// clip space exactly, for fullscreen passes.
Geometry quadGeometry(float side);
