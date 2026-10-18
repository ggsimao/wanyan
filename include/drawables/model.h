#pragma once

#include "drawables/basic_mesh.h"
#include "drawables/drawable.h"

#include <filesystem>
#include <vector>

// A model loaded from a file through Assimp: one BasicMesh per mesh in the
// file, each with its own material. Textures shared between meshes are
// loaded once.
class Model : public Drawable {
public:
    // Throws std::runtime_error if the file can't be loaded.
    explicit Model(const std::filesystem::path& path);

    void draw(const ShaderProgramBinding& shader, TextureUnits& units) const override;
    void drawInstanced(const ShaderProgramBinding& shader, TextureUnits& units,
                       GLsizei instances) const override;
    void setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const override;

    const std::vector<BasicMesh>& meshes() const;

private:
    std::vector<BasicMesh> _meshes;
};
