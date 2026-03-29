#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

#include "Window.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "SoundManager.hpp"
#include "Mesh.hpp"
#include "TextureManager.hpp"
#include "Random.hpp"
#include "../game/Scenario.hpp"
#include "../game/ScoreManager.hpp"
#include "../game/AppState.hpp"
#include "../scripting/LuaManager.hpp"

namespace klab {

struct InstanceData {
    glm::mat4 model;
    glm::vec3 color;
    glm::vec3 scale;
};

class Engine {
public:
    Engine();
    ~Engine();

    void run();

private:
    void initResources();
    void processInput(float deltaTime);
    void update(float deltaTime);
    void render(float fpsDisplay);
    
    void setupInstancedVAO(MeshData& mesh);
    void updateInstanceBuffer(const std::vector<InstanceData>& data);
    void drawTargetInstances(TargetShape shape, MeshData& mesh);

    Window m_window;
    Renderer m_renderer;
    Camera m_camera;
    AppState m_state;
    Scenario m_scenario;
    LuaManager m_lua;
    ParticleSystem m_particles;

    GLuint m_instanceVBO;

    // Loop state
    bool m_leftMousePressed = false;
    float m_lastFrame = 0.0f;
    float m_physicsAccumulator = 0.0f;
    const float m_fixedDeltaTime = 1.0f / 120.0f;
};

} // namespace klab
