#include "Renderer.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace klab {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(m_program);
}

void Shader::use() const {
    glUseProgram(m_program);
}

GLint Shader::getUniformLocation(const std::string& name) const {
    if (m_locationCache.find(name) != m_locationCache.end()) {
        return m_locationCache[name];
    }
    GLint location = glGetUniformLocation(m_program, name.c_str());
    m_locationCache[name] = location;
    return location;
}

void Shader::setUniform(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), (int)value);
}

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed (" << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "): " << infoLog << std::endl;
    }
    return shader;
}

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    
    glGenBuffers(1, &m_matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_matricesUBO);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &m_matricesUBO);
}

void Renderer::clear() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setMatrices(const glm::mat4& projection, const glm::mat4& view) const {
    glBindBuffer(GL_UNIFORM_BUFFER, m_matricesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// Particle System Implementation
ParticleSystem::ParticleSystem(int maxParticles) : m_maxParticles(maxParticles) {
    m_particles.reserve(maxParticles);
}

void ParticleSystem::update(float deltaTime) {
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->life -= deltaTime * 2.0f; // Fast fade
        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
        } else {
            it->position += it->velocity * deltaTime;
            it->velocity.y -= 9.8f * deltaTime * 0.5f; // Slight gravity
            ++it;
        }
    }
}

void ParticleSystem::emit(glm::vec3 pos, glm::vec3 color, int count) {
    for (int i = 0; i < count; i++) {
        if ((int)m_particles.size() >= m_maxParticles) return;

        Particle p;
        p.position = pos;
        
        // Random velocity
        float vx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float vy = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float vz = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        p.velocity = glm::vec3(vx, vy, vz) * 5.0f;
        
        p.color = color;
        p.life = 1.0f;
        p.size = 0.1f + ((float)rand() / RAND_MAX) * 0.2f;
        m_particles.push_back(p);
    }
}

} // namespace klab
