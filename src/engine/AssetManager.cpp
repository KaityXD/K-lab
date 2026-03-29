#include "AssetManager.hpp"
#include "TextureManager.hpp"
#include <iostream>

namespace klab {

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager() {
    clear();
}

void AssetManager::clear() {
    for (auto& entry : m_shaderEntries) {
        entry.shader.reset();
        entry.generation++;
    }
    m_shaderEntries.clear();
    m_shaderLookup.clear();

    for (auto& entry : m_textureEntries) {
        if (entry.id != 0) glDeleteTextures(1, &entry.id);
        entry.generation++;
    }
    m_textureEntries.clear();
    m_textureLookup.clear();

    if (m_checkerboardTexture != 0) {
        glDeleteTextures(1, &m_checkerboardTexture);
        m_checkerboardTexture = 0;
    }

    for (auto& entry : m_meshEntries) {
        glDeleteVertexArrays(1, &entry.mesh.VAO);
        glDeleteVertexArrays(1, &entry.mesh.VAO_instanced);
        glDeleteBuffers(1, &entry.mesh.VBO);
        glDeleteBuffers(1, &entry.mesh.EBO);
        entry.generation++;
    }
    m_meshEntries.clear();
    m_meshLookup.clear();
}

ShaderHandle AssetManager::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    if (m_shaderLookup.find(name) != m_shaderLookup.end()) {
        uint32_t id = m_shaderLookup[name];
        return ShaderHandle(id + 1, m_shaderEntries[id].generation);
    }
    
    std::string v = readFile(vertPath);
    std::string f = readFile(fragPath);
    if (v.empty() || f.empty()) return ShaderHandle();

    auto shader = std::make_shared<Shader>(v, f);
    
    // Bind GlobalMatrices to point 0
    GLuint blockIndex = glGetUniformBlockIndex(shader->getProgramId(), "GlobalMatrices");
    if (blockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(shader->getProgramId(), blockIndex, 0);
    }

    uint32_t id = (uint32_t)m_shaderEntries.size();
    m_shaderEntries.push_back({shader, 1, name});
    m_shaderLookup[name] = id;
    
    return ShaderHandle(id + 1, 1);
}

ShaderHandle AssetManager::getShaderHandle(const std::string& name) {
    auto it = m_shaderLookup.find(name);
    if (it != m_shaderLookup.end()) {
        uint32_t id = it->second;
        return ShaderHandle(id + 1, m_shaderEntries[id].generation);
    }
    return ShaderHandle();
}

std::shared_ptr<Shader> AssetManager::getShader(const std::string& name) {
    auto it = m_shaderLookup.find(name);
    if (it != m_shaderLookup.end()) return m_shaderEntries[it->second].shader;
    return nullptr;
}

std::shared_ptr<Shader> AssetManager::getShader(ShaderHandle handle) {
    if (!handle.isValid()) return nullptr;
    uint32_t index = handle.getId() - 1;
    if (index >= m_shaderEntries.size()) return nullptr;
    if (m_shaderEntries[index].generation != handle.getGeneration()) return nullptr;
    return m_shaderEntries[index].shader;
}

TextureHandle AssetManager::loadTexture(const std::string& path) {
    if (m_textureLookup.find(path) != m_textureLookup.end()) {
        uint32_t id = m_textureLookup[path];
        return TextureHandle(id + 1, m_textureEntries[id].generation);
    }

    unsigned int texture = klab::loadTexture(path.c_str());
    if (texture == 0) return TextureHandle();

    uint32_t id = (uint32_t)m_textureEntries.size();
    m_textureEntries.push_back({texture, 1, path});
    m_textureLookup[path] = id;
    
    return TextureHandle(id + 1, 1);
}

TextureHandle AssetManager::getTextureHandle(const std::string& path) {
    auto it = m_textureLookup.find(path);
    if (it != m_textureLookup.end()) {
        uint32_t id = it->second;
        return TextureHandle(id + 1, m_textureEntries[id].generation);
    }
    return loadTexture(path);
}

unsigned int AssetManager::getTexture(const std::string& path) {
    auto handle = getTextureHandle(path);
    return getTexture(handle);
}

unsigned int AssetManager::getTexture(TextureHandle handle) {
    if (!handle.isValid()) return 0;
    uint32_t index = handle.getId() - 1;
    if (index >= m_textureEntries.size()) return 0;
    if (m_textureEntries[index].generation != handle.getGeneration()) return 0;
    return m_textureEntries[index].id;
}

unsigned int AssetManager::getCheckerboardTexture() {
    if (m_checkerboardTexture == 0) {
        m_checkerboardTexture = generateCheckerboardTexture();
    }
    return m_checkerboardTexture;
}

MeshHandle AssetManager::loadMesh(const std::string& name, MeshData mesh) {
    if (m_meshLookup.find(name) != m_meshLookup.end()) {
        uint32_t id = m_meshLookup[name];
        m_meshEntries[id].mesh = mesh; // Replace
        m_meshEntries[id].generation++;
        return MeshHandle(id + 1, m_meshEntries[id].generation);
    }

    uint32_t id = (uint32_t)m_meshEntries.size();
    m_meshEntries.push_back({mesh, 1, name});
    m_meshLookup[name] = id;
    
    return MeshHandle(id + 1, 1);
}

MeshHandle AssetManager::getMeshHandle(const std::string& name) {
    auto it = m_meshLookup.find(name);
    if (it != m_meshLookup.end()) {
        uint32_t id = it->second;
        return MeshHandle(id + 1, m_meshEntries[id].generation);
    }
    return MeshHandle();
}

MeshData& AssetManager::getMesh(const std::string& name) {
    uint32_t id = m_meshLookup.at(name);
    return m_meshEntries[id].mesh;
}

MeshData& AssetManager::getMesh(MeshHandle handle) {
    uint32_t index = handle.getId() - 1;
    return m_meshEntries[index].mesh;
}

} // namespace klab
