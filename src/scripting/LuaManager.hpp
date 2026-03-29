#pragma once
#include <string>
#include <lua.hpp>
#include <glm/glm.hpp>
#include <functional>

// Define LUA_OK for Lua 5.1 compatibility
#ifndef LUA_OK
#define LUA_OK 0
#endif

namespace klab {

class Scenario;

class LuaManager {
public:
    LuaManager();
    ~LuaManager();

    bool loadFile(const std::string& path);
    void update(float deltaTime);
    void onStart();
    void onHit(float x, float y, float z);
    void onMiss();
    void clearCallbacks();
    void setScenario(Scenario* sc);
    void setParticleSystem(class ParticleSystem* ps) { m_particleSystem = ps; }
    void setCameraData(glm::vec3 pos, glm::vec3 front) { m_cameraPos = pos; m_cameraFront = front; }
    void setPlayerPosCallback(std::function<void(float, float, float)> cb) { m_setPlayerPosCallback = cb; }
    void setRandomSeed(uint32_t seed);

    static std::string getGlobalString(const std::string& path, const std::string& varName);
    static std::vector<std::string> getGlobalStrings(const std::string& path, const std::string& varName);

    static int l_spawnTarget(lua_State* L);
    static int l_spawnWall(lua_State* L);
    static int l_clearTargets(lua_State* L);
    static int l_clearWalls(lua_State* L);
    static int l_getTime(lua_State* L);
    static int l_getTargetCount(lua_State* L);
    static int l_getTargets(lua_State* L);
    static int l_getScore(lua_State* L);
    static int l_getHits(lua_State* L);
    static int l_getMisses(lua_State* L);
    static int l_getTotalShots(lua_State* L);
    static int l_setScore(lua_State* L);
    static int l_setHits(lua_State* L);
    static int l_setMisses(lua_State* L);
    static int l_setTotalShots(lua_State* L);
    static int l_setAutoShoot(lua_State* L);
    static int l_setIsAuto(lua_State* L);
    static int l_setFireRate(lua_State* L);
    static int l_setCooldown(lua_State* L);
    static int l_setTargetsPersistent(lua_State* L);
    static int l_setTimeOut(lua_State* L);
    static int l_setTargetSize(lua_State* L);
    static int l_setTargetColor(lua_State* L);
    static int l_setTargetHealth(lua_State* L);
    static int l_setTargetHealthBar(lua_State* L);
    static int l_setTargetShape(lua_State* L);
    static int l_removeTarget(lua_State* L);

    static int l_setTargetVelocity(lua_State* L);
    static int l_getTargetVelocity(lua_State* L);
    static int l_setTargetGravity(lua_State* L);
    static int l_setTargetScale(lua_State* L);
    static int l_setTargetInvincible(lua_State* L);
    static int l_setTargetPosition(lua_State* L);
    static int l_setPlayerPos(lua_State* L);
    static int l_getCameraPos(lua_State* L);
    static int l_getCameraFront(lua_State* L);

    // Audio
    static int l_playSound(lua_State* L);
    static int l_playSoundAt(lua_State* L);

    // Visuals
    static int l_setSkyColor(lua_State* L);
    static int l_setFloorColor(lua_State* L);
    static int l_setCrosshair(lua_State* L);

    // Particles
    static int l_emitParticles(lua_State* L);

    // Input
    static int l_isKeyDown(lua_State* L);
    static int l_isMouseDown(lua_State* L);

    // Random functions for deterministic replay support  
    static int l_setRandomSeed(lua_State* L);
    static int l_getRandomSeed(lua_State* L);
    static int l_randomFloat(lua_State* L);
    static int l_randomInt(lua_State* L);
    static int l_randomBool(lua_State* L);
    static int l_randomGaussian(lua_State* L);
    static int l_randomPoisson(lua_State* L);
    static int l_randomElement(lua_State* L);

private:
    lua_State* L;
    static LuaManager* s_instance;
    class ParticleSystem* m_particleSystem = nullptr;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraFront;
    std::function<void(float, float, float)> m_setPlayerPosCallback;
};

} // namespace klab
