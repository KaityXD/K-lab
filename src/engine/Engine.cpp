#include "Engine.hpp"
#include "AssetManager.hpp"
#include "../ui/Menu.hpp"
#include "../ui/Settings.hpp"
#include "../ui/Crosshair.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <ctime>
#include <glm/gtc/type_ptr.hpp>

namespace klab {

float calculateDegreesPerCount(float cm360, float dpi) {
    if (cm360 <= 0 || dpi <= 0) return 0;
    return (360.0f / cm360) * (2.54f / dpi);
}

Engine::Engine() 
    : m_window(1280, 720, "K-Lab - Aim Trainer"),
      m_particles(2000)
{
    Input::init(m_window.getNativeWindow());
    m_lua.setScenario(&m_scenario);
    m_lua.setPlayerPosCallback([&](float x, float y, float z) {
        m_camera.setPosition(glm::vec3(x, y, z));
    });

    // Initialize random seed for deterministic replay support
    uint32_t seed = static_cast<uint32_t>(std::time(nullptr));
    Random::getInstance().setSeed(seed);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window.getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    initResources();

    AppState::setInstance(&m_state);
    SettingsManager::refreshScenarios(m_state);
    SettingsManager::refreshSounds(m_state);
    SettingsManager::refreshTextures(m_state);
    SettingsManager::loadCrosshairs(m_state);
    SettingsManager::loadConfig(m_state);

    // Initialize random seed for deterministic replay support
    uint32_t currentSeed = static_cast<uint32_t>(std::time(nullptr));
    Random::getInstance().setSeed(currentSeed);
    
    // Pass the seed to Lua for deterministic scenarios
    m_lua.setRandomSeed(currentSeed);

    for (size_t i = 0; i < m_state.soundFiles.size(); ++i) {
        if (m_state.soundFiles[i] == m_state.hitSound) m_state.currentHitSound = (int)i;
        if (m_state.soundFiles[i] == m_state.missSound) m_state.currentMissSound = (int)i;
    }
    for (size_t i = 0; i < m_state.textureFiles.size(); ++i) {
        if (m_state.textureFiles[i] == m_state.currentWallTexturePath) {
            m_state.currentWallTextureIdx = (int)i;
            break;
        }
    }
    if (m_state.currentWallTextureIdx > 0 && m_state.currentWallTextureIdx < (int)m_state.textureFiles.size()) {
        m_state.wallTexture = loadTexture(m_state.textureFiles[m_state.currentWallTextureIdx].c_str());
    } else {
        m_state.wallTexture = generateCheckerboardTexture();
    }

    ScoreManager::getInstance().load();
    SoundManager::getInstance().setHitSound(m_state.hitSound);
    SoundManager::getInstance().setMissSound(m_state.missSound);
    SoundManager::getInstance().setHitSoundEnabled(m_state.hitEnabled);
    SoundManager::getInstance().setMissSoundEnabled(m_state.missEnabled);

    if (m_state.fullscreen) m_window.toggleFullscreen();
    if (!m_state.scenarioFiles.empty() && m_state.currentScenario < (int)m_state.scenarioFiles.size()) {
        m_scenario.reset();
        m_lua.loadFile(m_state.scenarioFiles[m_state.currentScenario]);
        m_lua.onStart();
    }
}

Engine::~Engine() {
    SettingsManager::saveConfig(m_state);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteBuffers(1, &m_instanceVBO);
}

void Engine::initResources() {
    auto& assets = AssetManager::getInstance();
    assets.loadShader("basic", "shaders/basic.vert", "shaders/basic.frag");
    assets.loadShader("instanced", "shaders/instanced.vert", "shaders/instanced.frag");
    assets.loadShader("outline", "shaders/outline.vert", "shaders/outline.frag");

    assets.loadMesh("box", createBox());
    assets.loadMesh("sphere", createSphere(32, 16));
    assets.loadMesh("bean", createBean(32, 16));
    assets.loadMesh("cone", createCone(32));
    assets.loadMesh("quad", createQuad());

    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 2000 * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

    setupInstancedVAO(assets.getMesh("box"));
    setupInstancedVAO(assets.getMesh("sphere"));
    setupInstancedVAO(assets.getMesh("bean"));
    setupInstancedVAO(assets.getMesh("cone"));
    setupInstancedVAO(assets.getMesh("quad"));
}

void Engine::setupInstancedVAO(MeshData& mesh) {
    glGenVertexArrays(1, &mesh.VAO_instanced);
    glBindVertexArray(mesh.VAO_instanced);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, model) + sizeof(float) * 4 * i));
        glVertexAttribDivisor(3 + i, 1);
    }
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, color)));
    glVertexAttribDivisor(7, 1);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(offsetof(InstanceData, scale)));
    glVertexAttribDivisor(8, 1);
    glBindVertexArray(0);
}

void Engine::updateInstanceBuffer(const std::vector<InstanceData>& data) {
    static size_t currentBufferSize = 2000;
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    if (data.size() > currentBufferSize) {
        currentBufferSize = data.size() + 500;
        glBufferData(GL_ARRAY_BUFFER, currentBufferSize * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
    }
    if (!data.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(InstanceData), data.data());
    }
}

void Engine::run() {
    float fpsDisplay = 0.0f;
    float fpsTimer = 0.0f;
    int fpsFrames = 0;
    double lastFrameTime = glfwGetTime();

    while (!m_window.shouldClose()) {
        // 1. Frame Capping (at the start for lower input latency)
        if (m_state.fpsTarget > 0) {
            double targetTime = 1.0 / m_state.fpsTarget;
            while (glfwGetTime() < lastFrameTime + targetTime) {
                // Busy wait for precision
            }
        }

        double currentFrame = glfwGetTime();
        float deltaTime = (float)(currentFrame - lastFrameTime);
        lastFrameTime = currentFrame;

        if (deltaTime > 0.25f) deltaTime = 0.25f;

        // 2. Poll events as late as possible before processing
        m_window.pollEvents();
        
        processInput(deltaTime);
        update(deltaTime);

        fpsTimer += deltaTime;
        fpsFrames++;
        if (fpsTimer >= 0.5f) {
            fpsDisplay = fpsFrames / fpsTimer;
            fpsTimer = 0.0f;
            fpsFrames = 0;
        }

        render(fpsDisplay);
        m_window.swapBuffers();
    }
}

void Engine::processInput(float deltaTime) {
    float currentFrame = (float)glfwGetTime();

    if (Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        static float lastEscapeTime = 0;
        if (currentFrame - lastEscapeTime > 0.2f) {
            m_state.menuOpen = !m_state.menuOpen;
            lastEscapeTime = currentFrame;
        }
    }
    if (!m_state.menuOpen && Input::isKeyPressed(GLFW_KEY_R)) {
        static float lastResetTime = 0;
        if (currentFrame - lastResetTime > 0.5f) {
            SettingsManager::resetScenario(m_scenario, m_lua, m_state);
            lastResetTime = currentFrame;
        }
    }
    if (m_state.menuOpen != m_state.wasMenuOpen) {
        Input::setCursorVisible(m_state.menuOpen);
        if (m_state.menuOpen) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        } else {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        m_state.wasMenuOpen = m_state.menuOpen;
    }

    if (!m_state.menuOpen) {
        double offsetX, offsetY;
        Input::getMouseOffset(offsetX, offsetY);
        float degPerCount = calculateDegreesPerCount(m_state.cm360, m_state.dpi);
        m_state.yaw += (float)offsetX * degPerCount;
        m_state.pitch += (float)offsetY * degPerCount;
        m_camera.update(m_state.yaw, m_state.pitch);
        SoundManager::getInstance().updateListener(m_camera.getPosition(), m_camera.getFront());
        m_lua.setCameraData(m_camera.getPosition(), m_camera.getFront());
    }
}

void Engine::update(float deltaTime) {
    if (!m_state.menuOpen) {
        m_physicsAccumulator += deltaTime;
        while (m_physicsAccumulator >= m_fixedDeltaTime) {
            m_lua.update(m_fixedDeltaTime);
            m_scenario.update(m_fixedDeltaTime);
            m_physicsAccumulator -= m_fixedDeltaTime;
        }
        
        m_state.displayedScore = (float)m_scenario.getScore();
        m_particles.update(deltaTime);

        for (auto it = m_state.hitmarkers.begin(); it != m_state.hitmarkers.end(); ) {
            it->life -= deltaTime * 3.0f;
            if (it->life <= 0) it = m_state.hitmarkers.erase(it);
            else ++it;
        }
        for (auto it = m_state.killfeed.begin(); it != m_state.killfeed.end(); ) {
            it->life -= deltaTime * 0.5f;
            if (it->life <= 0) it = m_state.killfeed.erase(it);
            else ++it;
        }

        if (m_scenario.getTimeRemaining() <= 0) {
            if (!m_state.menuOpen && !m_scenario.hasBeenSaved() && m_scenario.getTotalShots() > 0 && m_state.currentScenario < (int)m_state.scenarioFiles.size()) {
                SettingsManager::saveCurrentScore(m_scenario, m_state);
            }
            m_state.menuOpen = true;
        }

        bool isMouseDown = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool isFirstClick = isMouseDown && !m_leftMousePressed;
        bool shouldShoot = m_scenario.getIsAuto() ? isMouseDown : isFirstClick;
        
        if (shouldShoot && m_scenario.canShoot()) {
            bool isAutoFiring = m_scenario.getIsAuto() && !isFirstClick;
            m_scenario.recordShot();
            if (m_scenario.checkHits(m_camera.getPosition(), m_camera.getFront(), isAutoFiring)) {
                glm::vec3 hp = m_scenario.getLastHitPos();
                m_lua.onHit(hp.x, hp.y, hp.z);
                float pitchOffset = m_scenario.getIsAuto() ? (float)(m_state.hitStreak * 50) : 0.0f;
                if (m_state.hitPitchRandomization > 0) {
                    pitchOffset += ((float)rand() / RAND_MAX * 2.0f - 1.0f) * m_state.hitPitchRandomization;
                }
                SoundManager::getInstance().playHitAt(hp, pitchOffset);
                if (m_scenario.getIsAuto()) m_state.hitStreak = std::min(m_state.hitStreak + 1, 23);

                if (m_state.showParticles) m_particles.emit(hp, glm::make_vec3(m_state.targetColor), 15);
                if (m_state.showHitmarkers) m_state.hitmarkers.push_back({ 1.0f, hp });
                if (m_state.showKillfeed) {
                    std::string msg = m_scenario.wasLastHitKill() ? ("Killed " + m_scenario.getLastHitName()) : "+10";
                    m_state.killfeed.insert(m_state.killfeed.begin(), { msg, 1.5f });
                    if (m_state.killfeed.size() > 5) m_state.killfeed.pop_back();
                }
            } else if (!m_leftMousePressed || m_scenario.isAutoShoot()) {
                m_lua.onMiss();
                SoundManager::getInstance().playMiss();
                m_state.hitStreak = 0;
            }
        }
        m_leftMousePressed = isMouseDown;
    }
}

void Engine::render(float fpsDisplay) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (m_state.menuOpen) {
        Menu::render(m_state, m_scenario, m_lua, m_window);
    } else {
        Menu::renderHUD(m_state, m_scenario, m_window, fpsDisplay);
        drawCrosshair(m_window, m_state.crosshair);
    }

    m_camera.updateFrustum(m_state.fov, (float)m_window.getWidth() / m_window.getHeight());

    glClearColor(m_state.skyColor[0], m_state.skyColor[1], m_state.skyColor[2], 1.0f);
    m_renderer.clear();
    
    glm::mat4 projection = m_camera.getProjectionMatrix(m_state.fov, (float)m_window.getWidth() / m_window.getHeight());
    glm::mat4 view = m_camera.getViewMatrix();
    m_renderer.setMatrices(projection, view);

    auto& assets = AssetManager::getInstance();
    auto instancedShader = assets.getShader("instanced");

    // 1. Static Floor
    instancedShader->use();
    instancedShader->setUniform("lightPos", glm::vec3(0.0f, 10.0f, 5.0f));
    instancedShader->setUniform("viewPos", m_camera.getPosition());
    instancedShader->setUniform("simpleVisuals", m_state.simpleShading);
    instancedShader->setUniform("useTexture", true);
    instancedShader->setUniform("useTiling", m_state.useTiling);
    instancedShader->setUniform("floorTiling", m_state.floorTiling);
    instancedShader->setUniform("wallTileScale", m_state.wallTileScale);
    instancedShader->setUniform("floorTileScale", m_state.floorTileScale);
    instancedShader->setUniform("isFloor", true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, assets.getCheckerboardTexture());
    instancedShader->setUniform("tex", 0);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    glm::vec3 floorScale(100.0f, 0.1f, 100.0f);
    model = glm::scale(model, floorScale);
    
    InstanceData floorData = { model, glm::make_vec3(m_state.floorColor), floorScale };
    updateInstanceBuffer({ floorData });
    
    glBindVertexArray(assets.getMesh("box").VAO_instanced);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 1);
    instancedShader->setUniform("isFloor", false);

    // 2. Walls (Instanced)
    instancedShader->use();
    instancedShader->setUniform("lightPos", glm::vec3(0.0f, 10.0f, 5.0f));
    instancedShader->setUniform("viewPos", m_camera.getPosition());
    instancedShader->setUniform("simpleVisuals", m_state.simpleShading);
    instancedShader->setUniform("useTexture", true);
    instancedShader->setUniform("useTiling", m_state.useTiling);
    instancedShader->setUniform("wallTileScale", m_state.wallTileScale);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_state.wallTexture);
    instancedShader->setUniform("tex", 0);

    std::vector<InstanceData> wallData;
    for (const auto& wall : m_scenario.getWalls()) {
        float radius = glm::length(wall.size) * 0.5f;
        if (!m_camera.isInsideFrustum(wall.position, radius)) continue;

        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, wall.position);
        m = glm::rotate(m, glm::radians(wall.rotation.x), glm::vec3(1, 0, 0));
        m = glm::rotate(m, glm::radians(wall.rotation.y), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(wall.rotation.z), glm::vec3(0, 0, 1));
        m = glm::scale(m, wall.size);
        wallData.push_back({m, wall.color, wall.size});
    }
    if (!wallData.empty()) {
        updateInstanceBuffer(wallData);
        glBindVertexArray(assets.getMesh("box").VAO_instanced);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, (GLsizei)wallData.size());
    }

    // 3. Targets (Instanced)
    instancedShader->setUniform("useTexture", false);
    drawTargetInstances(TargetShape::SPHERE, assets.getMesh("sphere"));
    drawTargetInstances(TargetShape::BEAN, assets.getMesh("bean"));
    drawTargetInstances(TargetShape::CONE, assets.getMesh("cone"));
    drawTargetInstances(TargetShape::BOX, assets.getMesh("box"));

    if (m_state.targetOutline) {
        auto outlineShader = assets.getShader("outline");
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        outlineShader->use();
        outlineShader->setUniform("outlineColor", glm::make_vec3(m_state.targetOutlineColor));
        outlineShader->setUniform("outlineWidth", m_state.targetOutlineWidth);
        
        for (const auto& target : m_scenario.getTargets()) {
            if (target->isActive()) {
                float totalRadius = target->getRadius() * glm::length(target->getScale());
                if (!m_camera.isInsideFrustum(target->getPosition(), totalRadius)) continue;

                model = glm::mat4(1.0f);
                model = glm::translate(model, target->getPosition());
                glm::vec3 targetScale = glm::vec3(target->getRadius() * 2.0f) * target->getScale();
                model = glm::scale(model, targetScale);
                outlineShader->setUniform("model", model);
                
                MeshData* mesh = &assets.getMesh("sphere");
                if (target->getShape() == TargetShape::BEAN) mesh = &assets.getMesh("bean");
                else if (target->getShape() == TargetShape::CONE) mesh = &assets.getMesh("cone");
                else if (target->getShape() == TargetShape::BOX) mesh = &assets.getMesh("box");
                
                glBindVertexArray(mesh->VAO);
                glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
            }
        }
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
    }

    // 4. Health Bars
    bool hasAnyHealthBar = false;
    for (const auto& target : m_scenario.getTargets()) { if (target->isActive() && target->hasHealthBar() && m_state.showHealthBars) { hasAnyHealthBar = true; break; } }

    if (hasAnyHealthBar) {
        instancedShader->use();
        instancedShader->setUniform("useTexture", false);
        instancedShader->setUniform("simpleVisuals", true);
        instancedShader->setUniform("isUI", true);
        
        glm::vec3 camRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
        glm::vec3 camUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
        
        for (const auto& target : m_scenario.getTargets()) {
            if (target->isActive() && target->hasHealthBar()) {
                if (!m_camera.isInsideFrustum(target->getPosition(), target->getRadius() * 2.0f)) continue;

                float healthPercent = target->getHealth() / target->getMaxHealth();
                float targetHeight = target->getRadius() * target->getScale().y;
                if (target->getShape() == TargetShape::BEAN) targetHeight *= 2.0f;
                
                glm::vec3 barPos = target->getPosition() + glm::vec3(0, targetHeight + m_state.healthBarOffset, 0);
                
                auto drawBar = [&](glm::vec3 pos, float width, float height, glm::vec3 color) {
                    glm::mat4 m = glm::mat4(1.0f);
                    m[0] = glm::vec4(camRight * width, 0.0f);
                    m[1] = glm::vec4(camUp * height, 0.0f);
                    m[2] = glm::vec4(m_camera.getFront(), 0.0f);
                    m[3] = glm::vec4(pos, 1.0f);
                    
                    InstanceData barData = { m, color, glm::vec3(1.0f) };
                    updateInstanceBuffer({ barData });
                    glBindVertexArray(assets.getMesh("quad").VAO_instanced);
                    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);
                };

                drawBar(barPos, m_state.healthBarWidth, m_state.healthBarHeight, glm::make_vec3(m_state.healthBarColorBg)); 
                glm::vec3 hpColor = glm::mix(glm::make_vec3(m_state.healthBarColorLow), glm::make_vec3(m_state.healthBarColorHigh), healthPercent);
                float innerWidth = m_state.healthBarWidth * healthPercent;
                drawBar(barPos - camRight * (m_state.healthBarWidth - innerWidth) * 0.5f - m_camera.getFront() * 0.005f, innerWidth, m_state.healthBarHeight * 0.8f, hpColor);
            }
        }
        instancedShader->setUniform("isUI", false);
    }

    // 5. Particles
    if (m_state.showParticles && !m_particles.getParticles().empty()) {
        instancedShader->use();
        instancedShader->setUniform("simpleVisuals", true);
        instancedShader->setUniform("useTexture", false);
        std::vector<InstanceData> pData;
        for (const auto& p : m_particles.getParticles()) {
            if (!m_camera.isInsideFrustum(p.position, p.size)) continue;
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, p.position);
            m = glm::scale(m, glm::vec3(p.size));
            pData.push_back({m, p.color * p.life, glm::vec3(p.size)});
        }
        if (!pData.empty()) {
            updateInstanceBuffer(pData);
            glBindVertexArray(assets.getMesh("sphere").VAO_instanced);
            glDrawElementsInstanced(GL_TRIANGLES, assets.getMesh("sphere").indexCount, GL_UNSIGNED_INT, 0, (GLsizei)pData.size());
        }
    }

    // 6. Hitmarkers
    if (m_state.showHitmarkers && !m_state.hitmarkers.empty()) {
        instancedShader->use();
        instancedShader->setUniform("useTexture", false);
        instancedShader->setUniform("simpleVisuals", true);
        instancedShader->setUniform("isUI", true);
        glBindVertexArray(assets.getMesh("quad").VAO_instanced);
        glm::vec3 camRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
        glm::vec3 camUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
        for (const auto& hm : m_state.hitmarkers) {
            if (!m_camera.isInsideFrustum(hm.pos, 0.5f)) continue;
            float s = 0.3f * hm.life;
            auto drawHM = [&](float rot) {
                glm::mat4 r = glm::mat4(1.0f);
                r[0] = glm::vec4(camRight, 0.0f);
                r[1] = glm::vec4(camUp, 0.0f);
                r[2] = glm::vec4(m_camera.getFront(), 0.0f);
                r[3] = glm::vec4(hm.pos, 1.0f);
                r = glm::rotate(r, glm::radians(rot), m_camera.getFront());
                r = glm::scale(r, glm::vec3(s, 0.04f, 1.0f));
                
                InstanceData hmData = { r, glm::vec3(1, 1, 1), glm::vec3(1.0f) };
                updateInstanceBuffer({ hmData });
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);
            };
            drawHM(45.0f);
            drawHM(-45.0f);
        }
        instancedShader->setUniform("isUI", false);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Engine::drawTargetInstances(TargetShape shape, MeshData& mesh) {
    std::vector<InstanceData> targetData;
    for (const auto& target : m_scenario.getTargets()) {
        if (target->isActive() && target->getShape() == shape) {
            float totalRadius = target->getRadius() * glm::length(target->getScale());
            if (!m_camera.isInsideFrustum(target->getPosition(), totalRadius)) continue;

            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, target->getPosition());
            glm::vec3 targetScale = glm::vec3(target->getRadius() * 2.0f) * target->getScale();
            m = glm::scale(m, targetScale);
            
            glm::vec3 finalColor = target->getColor();
            if (m_state.targetFlash && target->getHitFlashTimer() > 0.0f) {
                finalColor = glm::mix(finalColor, glm::vec3(1.0f, 1.0f, 1.0f), 0.8f);
            }
            targetData.push_back({m, finalColor, targetScale});
        }
    }
    if (!targetData.empty()) {
        updateInstanceBuffer(targetData);
        glBindVertexArray(mesh.VAO_instanced);
        glDrawElementsInstanced(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0, (GLsizei)targetData.size());
    }
}

} // namespace klab
