#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <GL/glew.h>
#include "Renderer.hpp"
#include "Mesh.hpp"
#include "Handle.hpp"

namespace klab {

// Handle type aliases
using ShaderHandle = Handle<Shader>;
using TextureHandle = Handle<unsigned int>;
using MeshHandle = Handle<MeshData>;

class AssetManager {
public:
    static AssetManager& getInstance();
    ~AssetManager();

    // Shaders
    ShaderHandle loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    ShaderHandle getShaderHandle(const std::string& name);
    std::shared_ptr<Shader> getShader(const std::string& name);
    std::shared_ptr<Shader> getShader(ShaderHandle handle);

    // Textures
    TextureHandle loadTexture(const std::string& path);
    TextureHandle getTextureHandle(const std::string& path);
    unsigned int getTexture(const std::string& path);
    unsigned int getTexture(TextureHandle handle);
    unsigned int getCheckerboardTexture();

    // Meshes
    MeshHandle loadMesh(const std::string& name, MeshData mesh);
    MeshHandle getMeshHandle(const std::string& name);
    MeshData& getMesh(const std::string& name);
    MeshData& getMesh(MeshHandle handle);

    void clear();

private:
    AssetManager() = default;

    struct ShaderEntry {
        std::shared_ptr<Shader> shader;
        uint32_t generation;
        std::string name;
    };
    struct TextureEntry {
        unsigned int id;
        uint32_t generation;
        std::string path;
    };
    struct MeshEntry {
        MeshData mesh;
        uint32_t generation;
        std::string name;
    };

    std::vector<ShaderEntry> m_shaderEntries;
    std::vector<TextureEntry> m_textureEntries;
    std::vector<MeshEntry> m_meshEntries;

    std::map<std::string, uint32_t> m_shaderLookup;
    std::map<std::string, uint32_t> m_textureLookup;
    std::map<std::string, uint32_t> m_meshLookup;
    
    unsigned int m_checkerboardTexture = 0;
};

} // namespace klab
