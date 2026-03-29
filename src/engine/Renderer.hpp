#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace klab {

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    void use() const;
    void setUniform(const std::string& name, const glm::mat4& value) const;
    void setUniform(const std::string& name, const glm::vec3& value) const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, bool value) const;

    GLuint getProgramId() const { return m_program; }

private:
    GLuint m_program;
    mutable std::unordered_map<std::string, GLint> m_locationCache;
    GLint getUniformLocation(const std::string& name) const;
    GLuint compileShader(GLenum type, const std::string& source);
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    void clear() const;
    void setMatrices(const glm::mat4& projection, const glm::mat4& view) const;

private:
    GLuint m_matricesUBO;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float life; // 1.0 to 0.0
    float size;
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 1000);
    void update(float deltaTime);
    void emit(glm::vec3 pos, glm::vec3 color, int count = 20);
    const std::vector<Particle>& getParticles() const { return m_particles; }
    void clear() { m_particles.clear(); }

private:
    std::vector<Particle> m_particles;
    int m_maxParticles;
};

} // namespace klab
