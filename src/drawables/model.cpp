#include "drawables/model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace {
    // Everything only needed while loading: the model's directory (texture paths
    // in the file are relative to it) and the textures loaded so far.
    class ModelLoader {
    public:
        explicit ModelLoader(std::filesystem::path path)
            : _path(std::move(path)), _directory(_path.parent_path()) {}

        // Single use: hands its meshes over, so it can only be called on a
        // temporary, as in ModelLoader(path).load().
        std::vector<BasicMesh> load() && {
            Assimp::Importer importer;
            const aiScene* scene =
                importer.ReadFile(_path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
            if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
                throw std::runtime_error(std::format("Couldn't load model {}: {}", _path.string(),
                                        importer.GetErrorString()));
            }
            processNode(*scene->mRootNode, *scene);
            return std::move(_meshes);
            // The importer, and the scene it owns, are freed here; everything
            // needed has been copied into the meshes and textures.
        }

    private:
        // Like the Rust version, node transformations are ignored: every mesh is
        // placed as stored in the file.
        void processNode(const aiNode& node, const aiScene& scene) {
            for (unsigned i = 0; i < node.mNumMeshes; ++i) {
                _meshes.push_back(processMesh(*scene.mMeshes[node.mMeshes[i]], scene));
            }
            for (unsigned i = 0; i < node.mNumChildren; ++i) {
                processNode(*node.mChildren[i], scene);
            }
        }

        BasicMesh processMesh(const aiMesh& mesh, const aiScene& scene) {
            std::vector<Vertex> vertices;
            vertices.reserve(mesh.mNumVertices);
            const aiVector3D* texCoords = mesh.mTextureCoords[0];  // null if the mesh has none

            for (unsigned i = 0; i < mesh.mNumVertices; ++i) {
                const aiVector3D& p = mesh.mVertices[i];
                Vertex vertex(glm::vec3(p.x, p.y, p.z));
                if (mesh.HasNormals()) {
                    const aiVector3D& n = mesh.mNormals[i];
                    vertex.normal = glm::vec3(n.x, n.y, n.z);
                }
                if (texCoords) {
                    // Negated V, as in the Rust version, on top of aiProcess_FlipUVs
                    // and stb's vertical flip. The three flips together are what the
                    // shaders expect, so they're kept as they are.
                    vertex.texCoords = glm::vec3(texCoords[i].x, -texCoords[i].y, 0.0f);
                }
                vertices.push_back(vertex);
            }

            std::vector<GLuint> indices;
            indices.reserve(static_cast<std::size_t>(mesh.mNumFaces) * 3);
            for (unsigned f = 0; f < mesh.mNumFaces; ++f) {
                const aiFace& face = mesh.mFaces[f];
                indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
            }

            return BasicMesh(vertices, indices, loadMaterial(*scene.mMaterials[mesh.mMaterialIndex]));
        }

        Material loadMaterial(const aiMaterial& material) {
            std::vector<TexturePtr> diffuse =
                loadTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
            if (diffuse.empty()) {
                diffuse.push_back(colorTexture(material, AI_MATKEY_COLOR_DIFFUSE, TextureType::DIFFUSE));
            }
            std::vector<TexturePtr> specular =
                loadTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
            if (specular.empty()) {
                specular.push_back(
                    colorTexture(material, AI_MATKEY_COLOR_SPECULAR, TextureType::SPECULAR));
            }
            return Material(std::move(diffuse), std::move(specular), shininess(material));
        }

        std::vector<TexturePtr> loadTextures(const aiMaterial& material, aiTextureType aiType,
                                            TextureType type) {
            std::vector<TexturePtr> textures;
            for (unsigned i = 0; i < material.GetTextureCount(aiType); ++i) {
                aiString file;
                material.GetTexture(aiType, i, &file);
                textures.push_back(texture((_directory / file.C_Str()).lexically_normal(), type));
            }
            return textures;
        }

        // Loads a texture once per model; later requests share the same one.
        // Fixes the Rust version, which skipped already-loaded textures and left
        // the second mesh using them without any.
        TexturePtr texture(const std::filesystem::path& path, TextureType type) {
            const std::string key = std::format("{}:{}", static_cast<int>(type), path.string());
            if (auto it = _textures.find(key); it != _textures.end()) {
                return it->second;
            }
            auto loaded = std::make_shared<const Texture2D>(type, path, GL_REPEAT);
            _textures.emplace(key, loaded);
            return loaded;
        }

        static TexturePtr colorTexture(const aiMaterial& material, const char* key, unsigned type,
                                    unsigned index, TextureType textureType) {
            aiColor3D color(0.0f, 0.0f, 0.0f);  // black if the material has no such colour
            material.Get(key, type, index, color);
            return std::make_shared<const Texture2D>(
                Texture2D::fromColor(textureType, glm::vec3(color.r, color.g, color.b)));
        }

        static float shininess(const aiMaterial& material) {
            constexpr float DEFAULT_SHININESS = 32.0f;
            float value = 0.0f;
            if (material.Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS && value > 0.0f) {
                return std::max(value, 1.0f);
            }
            return DEFAULT_SHININESS;
        }

        // Declaration order matters: _directory is initialized from _path.
        std::filesystem::path _path;
        std::filesystem::path _directory;
        std::unordered_map<std::string, TexturePtr> _textures;
        std::vector<BasicMesh> _meshes;
    };
}  // namespace

Model::Model(const std::filesystem::path& path) : _meshes(ModelLoader(path).load()) {}

void Model::draw(const ShaderProgramBinding& shader, TextureUnits& units) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.draw(shader, units);
    }
}

void Model::drawPlain(const ShaderProgramBinding& shader) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.drawPlain(shader);
    }
}

void Model::drawVertices(const ShaderProgramBinding& shader) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.drawVertices(shader);
    }
}

void Model::drawInstanced(const ShaderProgramBinding& shader, TextureUnits& units,
                          GLsizei instances) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.drawInstanced(shader, units, instances);
    }
}

void Model::drawPlainInstanced(const ShaderProgramBinding& shader, GLsizei instances) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.drawPlainInstanced(shader, instances);
    }
}

void Model::drawVerticesInstanced(const ShaderProgramBinding& shader, GLsizei instances) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.drawVerticesInstanced(shader, instances);
    }
}

void Model::setupInstanceAttributes(const ArrayBuffer& instanceBuffer) const {
    for (const BasicMesh& mesh : _meshes) {
        mesh.setupInstanceAttributes(instanceBuffer);
    }
}

const std::vector<BasicMesh>& Model::meshes() const {
    return _meshes;
}
