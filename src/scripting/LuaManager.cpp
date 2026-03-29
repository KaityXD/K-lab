#include "LuaManager.hpp"
#include "../game/Scenario.hpp"
#include "../game/AppState.hpp"
#include "../engine/SoundManager.hpp"
#include "../engine/Input.hpp"
#include "../engine/Renderer.hpp"
#include "../engine/Random.hpp"
#include <iostream>

namespace klab {

LuaManager* LuaManager::s_instance = nullptr;
static Scenario* s_scenario = nullptr;

LuaManager::LuaManager() {
    L = luaL_newstate();
    luaL_openlibs(L);
    s_instance = this;

    // Register C++ functions to Lua
    lua_register(L, "spawnTarget", l_spawnTarget);
    lua_register(L, "spawnWall", l_spawnWall);
    lua_register(L, "clearTargets", l_clearTargets);
    lua_register(L, "clearWalls", l_clearWalls);
    lua_register(L, "getTime", l_getTime);
    lua_register(L, "getTargetCount", l_getTargetCount);
    lua_register(L, "getTargets", l_getTargets);
    lua_register(L, "getScore", l_getScore);
    lua_register(L, "getHits", l_getHits);
    lua_register(L, "getMisses", l_getMisses);
    lua_register(L, "getTotalShots", l_getTotalShots);
    lua_register(L, "setScore", l_setScore);
    lua_register(L, "setHits", l_setHits);
    lua_register(L, "setMisses", l_setMisses);
    lua_register(L, "setTotalShots", l_setTotalShots);
    lua_register(L, "setAutoShoot", l_setAutoShoot);
    lua_register(L, "setIsAuto", l_setIsAuto);
    lua_register(L, "setFireRate", l_setFireRate);
    lua_register(L, "setCooldown", l_setCooldown);
    lua_register(L, "setTargetsPersistent", l_setTargetsPersistent);
    lua_register(L, "setTimeOut", l_setTimeOut);
    lua_register(L, "setTargetSize", l_setTargetSize);
    lua_register(L, "setTargetColor", l_setTargetColor);
    lua_register(L, "setTargetHealth", l_setTargetHealth);
    lua_register(L, "setTargetHealthBar", l_setTargetHealthBar);
    lua_register(L, "setTargetShape", l_setTargetShape);
    lua_register(L, "removeTarget", l_removeTarget);
    
    lua_register(L, "setTargetVelocity", l_setTargetVelocity);
    lua_register(L, "getTargetVelocity", l_getTargetVelocity);
    lua_register(L, "setTargetGravity", l_setTargetGravity);
    lua_register(L, "setTargetScale", l_setTargetScale);
    lua_register(L, "setTargetInvincible", l_setTargetInvincible);
    lua_register(L, "setTargetPosition", l_setTargetPosition);
    lua_register(L, "setPlayerPos", l_setPlayerPos);
    lua_register(L, "getCameraPos", l_getCameraPos);
    lua_register(L, "getCameraFront", l_getCameraFront);
    
    // Audio
    lua_register(L, "playSound", l_playSound);
    lua_register(L, "playSoundAt", l_playSoundAt);

    // Visuals
    lua_register(L, "setSkyColor", l_setSkyColor);
    lua_register(L, "setFloorColor", l_setFloorColor);
    lua_register(L, "setCrosshair", l_setCrosshair);

    // Particles
    lua_register(L, "emitParticles", l_emitParticles);

    // Input
    lua_register(L, "isKeyDown", l_isKeyDown);
    lua_register(L, "isMouseDown", l_isMouseDown);
    
    // Random functions for deterministic replay support
    lua_register(L, "setRandomSeed", l_setRandomSeed);
    lua_register(L, "getRandomSeed", l_getRandomSeed);
    lua_register(L, "randomFloat", l_randomFloat);
    lua_register(L, "randomInt", l_randomInt);
    lua_register(L, "randomBool", l_randomBool);
    lua_register(L, "randomGaussian", l_randomGaussian);
    lua_register(L, "randomPoisson", l_randomPoisson);
    lua_register(L, "randomElement", l_randomElement);
    
    // Target Shapes
    lua_newtable(L);
    lua_pushinteger(L, (int)TargetShape::BOX);
    lua_setfield(L, -2, "BOX");
    lua_pushinteger(L, (int)TargetShape::SPHERE);
    lua_setfield(L, -2, "SPHERE");
    lua_pushinteger(L, (int)TargetShape::BEAN);
    lua_setfield(L, -2, "BEAN");
    lua_pushinteger(L, (int)TargetShape::CONE);
    lua_setfield(L, -2, "CONE");
    lua_setglobal(L, "TargetShape");
    
    // Override math.random with our seeded version for deterministic replays
    lua_getglobal(L, "math");
    lua_pushcfunction(L, [](lua_State* L) -> int {
        int n = lua_gettop(L);
        if (n == 0) {
            lua_pushnumber(L, randomFloat());
        } else if (n == 1) {
            int max = (int)luaL_checkinteger(L, 1);
            lua_pushinteger(L, randomInt(1, max));
        } else if (n == 2) {
            int min = (int)luaL_checkinteger(L, 1);
            int max = (int)luaL_checkinteger(L, 2);
            lua_pushinteger(L, randomInt(min, max));
        }
        return 1;
    });
    lua_setfield(L, -2, "random");
    
    lua_pushcfunction(L, [](lua_State* L) -> int {
        float min = (float)luaL_optnumber(L, 1, 0.0);
        float max = (float)luaL_optnumber(L, 2, 1.0);
        lua_pushnumber(L, randomFloat(min, max));
        return 1;
    });
    lua_setfield(L, -2, "randomFloat");
    
    lua_pop(L, 1); // pop math table
    
    lua_register(L, "print", [](lua_State* L) -> int {
        int n = lua_gettop(L);
        for (int i = 1; i <= n; i++) {
            const char* s = lua_tostring(L, i);
            if (s) std::cout << s << (i == n ? "" : "\t");
        }
        std::cout << std::endl;
        return 0;
    });
}

LuaManager::~LuaManager() {
    lua_close(L);
}

void LuaManager::setScenario(Scenario* sc) {
    s_scenario = sc;
}

bool LuaManager::loadFile(const std::string& path) {
    clearCallbacks();
    if (luaL_dofile(L, path.c_str()) != LUA_OK) {
        std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
        return false;
    }
    return true;
}

std::string LuaManager::getGlobalString(const std::string& path, const std::string& varName) {
    lua_State* tempL = luaL_newstate();
    luaL_openlibs(tempL);
    std::string result = "";
    if (luaL_dofile(tempL, path.c_str()) == LUA_OK) {
        lua_getglobal(tempL, varName.c_str());
        if (lua_isstring(tempL, -1)) {
            result = lua_tostring(tempL, -1);
        }
    }
    lua_close(tempL);
    return result;
}

std::vector<std::string> LuaManager::getGlobalStrings(const std::string& path, const std::string& varName) {
    lua_State* tempL = luaL_newstate();
    luaL_openlibs(tempL);
    std::vector<std::string> result;
    if (luaL_dofile(tempL, path.c_str()) == LUA_OK) {
        lua_getglobal(tempL, varName.c_str());
        if (lua_istable(tempL, -1)) {
            size_t n = lua_objlen(tempL, -1);
            for (size_t i = 1; i <= n; i++) {
                lua_rawgeti(tempL, -1, (int)i);
                if (lua_isstring(tempL, -1)) {
                    result.push_back(lua_tostring(tempL, -1));
                }
                lua_pop(tempL, 1);
            }
        }
    }
    lua_close(tempL);
    return result;
}

void LuaManager::clearCallbacks() {
    lua_pushnil(L); lua_setglobal(L, "onStart");
    lua_pushnil(L); lua_setglobal(L, "onUpdate");
    lua_pushnil(L); lua_setglobal(L, "onHit");
    lua_pushnil(L); lua_setglobal(L, "onMiss");
}

void LuaManager::onStart() {
    lua_getglobal(L, "onStart");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "Lua Start Error: " << lua_tostring(L, -1) << std::endl;
        }
    } else {
        lua_pop(L, 1);
    }
}

void LuaManager::update(float deltaTime) {
    lua_getglobal(L, "onUpdate");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, deltaTime);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            std::cerr << "Lua Update Error: " << lua_tostring(L, -1) << std::endl;
        }
    } else {
        lua_pop(L, 1);
    }
}

void LuaManager::onHit(float x, float y, float z) {
    lua_getglobal(L, "onHit");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_pushnumber(L, z);
        if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
            std::cerr << "Lua Hit Error: " << lua_tostring(L, -1) << std::endl;
        }
    } else {
        lua_pop(L, 1);
    }
}

void LuaManager::onMiss() {
    lua_getglobal(L, "onMiss");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            std::cerr << "Lua Miss Error: " << lua_tostring(L, -1) << std::endl;
        }
    } else {
        lua_pop(L, 1);
    }
}

int LuaManager::l_spawnTarget(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float z = luaL_checknumber(L, 3);
    float r = luaL_optnumber(L, 4, 0.5f);
    int s = luaL_optinteger(L, 5, (int)TargetShape::SPHERE);
    
    // Optional Color
    glm::vec3 color(1.0f, 0.0f, 0.0f);
    if (lua_istable(L, 6)) {
        lua_rawgeti(L, 6, 1); color.r = luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 6, 2); color.g = luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 6, 3); color.b = luaL_checknumber(L, -1); lua_pop(L, 1);
    }

    if (s_scenario) {
        float health = (float)luaL_optnumber(L, 9, 100.0f);
        bool showHP = lua_isboolean(L, 10) ? lua_toboolean(L, 10) : (health > 100.0f);
        std::string name = luaL_optstring(L, 11, "");
        
        s_scenario->spawnTarget(glm::vec3(x, y, z), r, (TargetShape)s, color, health, name);
        auto& lastTarget = s_scenario->getTargets().back();
        lastTarget->setHasHealthBar(showHP);
        
        // Optional Scale {x, y, z}
        if (lua_istable(L, 12)) {
            glm::vec3 scale(1.0f);
            lua_rawgeti(L, 12, 1); scale.x = luaL_checknumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, 12, 2); scale.y = luaL_checknumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, 12, 3); scale.z = luaL_checknumber(L, -1); lua_pop(L, 1);
            lastTarget->setScale(scale);
        }

        // Optional Velocity
        if (lua_istable(L, 7)) {
            glm::vec3 vel;
            lua_rawgeti(L, 7, 1); vel.x = luaL_checknumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, 7, 2); vel.y = luaL_checknumber(L, -1); lua_pop(L, 1);
            lua_rawgeti(L, 7, 3); vel.z = luaL_checknumber(L, -1); lua_pop(L, 1);
            s_scenario->getTargets().back()->setVelocity(vel);
        }

        // Optional Lifetime
        float lifetime = luaL_optnumber(L, 8, 0.0f);
        if (lifetime > 0) {
            s_scenario->getTargets().back()->setLifetime(lifetime);
        }
    }
    return 0;
}

int LuaManager::l_spawnWall(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    float sx = (float)luaL_checknumber(L, 4);
    float sy = (float)luaL_checknumber(L, 5);
    float sz = (float)luaL_checknumber(L, 6);
    
    glm::vec3 color(0.5f, 0.5f, 0.5f);
    if (lua_istable(L, 7)) {
        lua_rawgeti(L, 7, 1); color.r = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 7, 2); color.g = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 7, 3); color.b = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    }

    glm::vec3 rotation(0.0f);
    if (lua_istable(L, 8)) {
        lua_rawgeti(L, 8, 1); rotation.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 8, 2); rotation.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 8, 3); rotation.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    } else if (lua_isnumber(L, 8)) {
        rotation.x = (float)lua_tonumber(L, 8);
        rotation.y = (float)luaL_optnumber(L, 9, 0.0f);
        rotation.z = (float)luaL_optnumber(L, 10, 0.0f);
    }

    if (s_scenario) {
        s_scenario->spawnWall(glm::vec3(x, y, z), glm::vec3(sx, sy, sz), color, rotation);
    }
    return 0;
}

int LuaManager::l_clearWalls(lua_State* L) {
    if (s_scenario) {
        s_scenario->clearWalls();
    }
    return 0;
}

int LuaManager::l_clearTargets(lua_State* L) {
    if (s_scenario) {
        s_scenario->clearTargets();
    }
    return 0;
}

int LuaManager::l_getTime(lua_State* L) {
    if (s_scenario) {
        lua_pushnumber(L, s_scenario->getTimeRemaining());
        return 1;
    }
    return 0;
}

int LuaManager::l_getTargetCount(lua_State* L) {
    if (s_scenario) {
        lua_pushinteger(L, s_scenario->getTargetCount());
        return 1;
    }
    return 0;
}

int LuaManager::l_getTargets(lua_State* L) {
    if (!s_scenario) return 0;
    
    auto& targets = s_scenario->getTargets();
    lua_newtable(L);
    int index = 1;
    for (const auto& target : targets) {
        if (target->isActive()) {
            lua_pushinteger(L, index++);
            lua_newtable(L);
            
            glm::vec3 pos = target->getPosition();
            lua_pushnumber(L, pos.x); lua_setfield(L, -2, "x");
            lua_pushnumber(L, pos.y); lua_setfield(L, -2, "y");
            lua_pushnumber(L, pos.z); lua_setfield(L, -2, "z");
            lua_pushnumber(L, target->getRadius()); lua_setfield(L, -2, "radius");
            lua_pushnumber(L, target->getHealth()); lua_setfield(L, -2, "health");
            lua_pushnumber(L, target->getMaxHealth()); lua_setfield(L, -2, "maxHealth");
            
            lua_settable(L, -3);
        }
    }
    return 1;
}

int LuaManager::l_getScore(lua_State* L) {
    if (s_scenario) {
        lua_pushinteger(L, s_scenario->getScore());
        return 1;
    }
    return 0;
}

int LuaManager::l_getHits(lua_State* L) {
    if (s_scenario) {
        lua_pushinteger(L, s_scenario->getHits());
        return 1;
    }
    return 0;
}

int LuaManager::l_getMisses(lua_State* L) {
    if (s_scenario) {
        lua_pushinteger(L, s_scenario->getMisses());
        return 1;
    }
    return 0;
}

int LuaManager::l_getTotalShots(lua_State* L) {
    if (s_scenario) {
        lua_pushinteger(L, s_scenario->getTotalShots());
        return 1;
    }
    return 0;
}

int LuaManager::l_setScore(lua_State* L) {
    int score = (int)luaL_checkinteger(L, 1);
    if (s_scenario) s_scenario->setScore(score);
    return 0;
}

int LuaManager::l_setHits(lua_State* L) {
    int hits = (int)luaL_checkinteger(L, 1);
    if (s_scenario) s_scenario->setHits(hits);
    return 0;
}

int LuaManager::l_setMisses(lua_State* L) {
    int misses = (int)luaL_checkinteger(L, 1);
    if (s_scenario) s_scenario->setMisses(misses);
    return 0;
}

int LuaManager::l_setTotalShots(lua_State* L) {
    int shots = (int)luaL_checkinteger(L, 1);
    if (s_scenario) s_scenario->setTotalShots(shots);
    return 0;
}

int LuaManager::l_setAutoShoot(lua_State* L) {
    bool autoShoot = lua_toboolean(L, 1);
    if (s_scenario) s_scenario->setAutoShoot(autoShoot);
    return 0;
}

int LuaManager::l_setIsAuto(lua_State* L) {
    bool isAuto = lua_toboolean(L, 1);
    if (s_scenario) s_scenario->setIsAuto(isAuto);
    return 0;
}

int LuaManager::l_setFireRate(lua_State* L) {
    float rpm = luaL_checknumber(L, 1);
    if (s_scenario) s_scenario->setFireRate(rpm);
    return 0;
}

int LuaManager::l_setCooldown(lua_State* L) {
    float seconds = luaL_checknumber(L, 1);
    if (s_scenario) {
        if (seconds > 0) s_scenario->setFireRate(60.0f / seconds);
        else s_scenario->setFireRate(0);
    }
    return 0;
}

int LuaManager::l_setTargetsPersistent(lua_State* L) {
    bool persistent = lua_toboolean(L, 1);
    if (s_scenario) s_scenario->setTargetsPersistent(persistent);
    return 0;
}

int LuaManager::l_setTimeOut(lua_State* L) {
    float seconds = (float)luaL_checknumber(L, 1);
    if (s_scenario) s_scenario->setTimeRemaining(seconds);
    return 0;
}

int LuaManager::l_setTargetSize(lua_State* L) {
    float size = (float)luaL_checknumber(L, 1);
    if (s_scenario) {
        for (auto& target : s_scenario->getTargets()) {
            if (target->isActive()) {
                target->setRadius(size);
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetColor(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setColor(glm::vec3(r, g, b));
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetHealth(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float hp = (float)luaL_checknumber(L, 2);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setHealth(hp);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetHealthBar(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    bool show = lua_toboolean(L, 2);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setHasHealthBar(show);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetShape(lua_State* L) {
    int index = luaL_checkinteger(L, 1);
    int shape = luaL_checkinteger(L, 2);
    
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setShape((TargetShape)shape);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_removeTarget(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto it = targets.begin(); it != targets.end(); ++it) {
            if ((*it)->isActive()) {
                if (activeIdx == index) {
                    (*it)->setActive(false);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetVelocity(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setVelocity(glm::vec3(x, y, z));
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_getTargetVelocity(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    glm::vec3 vel = t->getVelocity();
                    lua_pushnumber(L, vel.x);
                    lua_pushnumber(L, vel.y);
                    lua_pushnumber(L, vel.z);
                    return 3;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetGravity(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float g = (float)luaL_checknumber(L, 2);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setGravity(g);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetScale(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setScale(glm::vec3(x, y, z));
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetInvincible(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    bool invincible = lua_toboolean(L, 2);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setInvincible(invincible);
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_setTargetPosition(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    if (s_scenario) {
        auto& targets = s_scenario->getTargets();
        int activeIdx = 1;
        for (auto& t : targets) {
            if (t->isActive()) {
                if (activeIdx == index) {
                    t->setPosition(glm::vec3(x, y, z));
                    break;
                }
                activeIdx++;
            }
        }
    }
    return 0;
}

int LuaManager::l_getCameraPos(lua_State* L) {
    if (s_instance) {
        lua_pushnumber(L, s_instance->m_cameraPos.x);
        lua_pushnumber(L, s_instance->m_cameraPos.y);
        lua_pushnumber(L, s_instance->m_cameraPos.z);
        return 3;
    }
    return 0;
}

int LuaManager::l_getCameraFront(lua_State* L) {
    if (s_instance) {
        lua_pushnumber(L, s_instance->m_cameraFront.x);
        lua_pushnumber(L, s_instance->m_cameraFront.y);
        lua_pushnumber(L, s_instance->m_cameraFront.z);
        return 3;
    }
    return 0;
}

int LuaManager::l_setPlayerPos(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    if (s_instance && s_instance->m_setPlayerPosCallback) {
        s_instance->m_setPlayerPosCallback(x, y, z);
    }
    return 0;
}

int LuaManager::l_playSound(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float pitchOffset = (float)luaL_optnumber(L, 2, 0.0f);
    SoundManager::getInstance().playSound(path, pitchOffset);
    return 0;
}

int LuaManager::l_playSoundAt(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);
    float pitchOffset = (float)luaL_optnumber(L, 5, 0.0f);
    SoundManager::getInstance().playSoundAt(path, glm::vec3(x, y, z), pitchOffset);
    return 0;
}

int LuaManager::l_setSkyColor(lua_State* L) {
    float r = (float)luaL_checknumber(L, 1);
    float g = (float)luaL_checknumber(L, 2);
    float b = (float)luaL_checknumber(L, 3);
    AppState* state = AppState::getInstance();
    if (state) {
        state->skyColor[0] = r;
        state->skyColor[1] = g;
        state->skyColor[2] = b;
    }
    return 0;
}

int LuaManager::l_setFloorColor(lua_State* L) {
    float r = (float)luaL_checknumber(L, 1);
    float g = (float)luaL_checknumber(L, 2);
    float b = (float)luaL_checknumber(L, 3);
    AppState* state = AppState::getInstance();
    if (state) {
        state->floorColor[0] = r;
        state->floorColor[1] = g;
        state->floorColor[2] = b;
    }
    return 0;
}

int LuaManager::l_setCrosshair(lua_State* L) {
    AppState* state = AppState::getInstance();
    if (!state || !lua_istable(L, 1)) return 0;

    auto& x = state->crosshair;
    
    lua_getfield(L, 1, "thickness"); if (lua_isnumber(L, -1)) x.thickness = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "length"); if (lua_isnumber(L, -1)) x.length = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "gap"); if (lua_isnumber(L, -1)) x.gap = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "dot"); if (lua_isboolean(L, -1)) x.dot = lua_toboolean(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "dotSize"); if (lua_isnumber(L, -1)) x.dotSize = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "circle"); if (lua_isboolean(L, -1)) x.circle = lua_toboolean(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "circleRadius"); if (lua_isnumber(L, -1)) x.circleRadius = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "outline"); if (lua_isboolean(L, -1)) x.outline = lua_toboolean(L, -1); lua_pop(L, 1);
    lua_getfield(L, 1, "outlineThickness"); if (lua_isnumber(L, -1)) x.outlineThickness = (float)lua_tonumber(L, -1); lua_pop(L, 1);
    
    lua_getfield(L, 1, "color");
    if (lua_istable(L, -1)) {
        for(int i=0; i<4; ++i) { lua_rawgeti(L, -1, i+1); x.color[i] = (float)luaL_optnumber(L, -1, x.color[i]); lua_pop(L, 1); }
    }
    lua_pop(L, 1);

    return 0;
}

int LuaManager::l_emitParticles(lua_State* L) {
    if (!s_instance || !s_instance->m_particleSystem) return 0;
    
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    float z = (float)luaL_checknumber(L, 3);
    
    glm::vec3 color(1.0f);
    if (lua_istable(L, 4)) {
        lua_rawgeti(L, 4, 1); color.r = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 4, 2); color.g = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, 4, 3); color.b = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
    }
    
    int count = (int)luaL_optinteger(L, 5, 20);
    s_instance->m_particleSystem->emit(glm::vec3(x, y, z), color, count);
    return 0;
}

int LuaManager::l_isKeyDown(lua_State* L) {
    int key = (int)luaL_checkinteger(L, 1);
    lua_pushboolean(L, Input::isKeyPressed(key));
    return 1;
}

int LuaManager::l_isMouseDown(lua_State* L) {
    int button = (int)luaL_optinteger(L, 1, 0); // Default Left
    lua_pushboolean(L, Input::isMouseButtonPressed(button));
    return 1;
}

// Random functions for deterministic replay support
int LuaManager::l_setRandomSeed(lua_State* L) {
    uint32_t seed = (uint32_t)luaL_checkinteger(L, 1);
    Random::getInstance().setSeed(seed);
    return 0;
}

int LuaManager::l_getRandomSeed(lua_State* L) {
    lua_pushinteger(L, Random::getInstance().getSeed());
    return 1;
}

int LuaManager::l_randomFloat(lua_State* L) {
    float min = (float)luaL_optnumber(L, 1, 0.0);
    float max = (float)luaL_optnumber(L, 2, 1.0);
    lua_pushnumber(L, Random::getInstance().getFloat(min, max));
    return 1;
}

int LuaManager::l_randomInt(lua_State* L) {
    int min = (int)luaL_checkinteger(L, 1);
    int max = (int)luaL_checkinteger(L, 2);
    lua_pushinteger(L, Random::getInstance().getInt(min, max));
    return 1;
}

int LuaManager::l_randomBool(lua_State* L) {
    float probability = (float)luaL_optnumber(L, 1, 0.5);
    lua_pushboolean(L, Random::getInstance().getBool(probability));
    return 1;
}

int LuaManager::l_randomGaussian(lua_State* L) {
    float mean = (float)luaL_optnumber(L, 1, 0.0);
    float stddev = (float)luaL_optnumber(L, 2, 1.0);
    lua_pushnumber(L, Random::getInstance().getGaussian(mean, stddev));
    return 1;
}

int LuaManager::l_randomPoisson(lua_State* L) {
    float lambda = (float)luaL_checknumber(L, 1);
    lua_pushinteger(L, Random::getInstance().getPoisson(lambda));
    return 1;
}

int LuaManager::l_randomElement(lua_State* L) {
    int n = lua_gettop(L);
    if (n < 1) return 0;
    
    int index = Random::getInstance().getInt(1, n);
    lua_pushvalue(L, index);
    return 1;
}

void LuaManager::setRandomSeed(uint32_t seed) {
    Random::getInstance().setSeed(seed);
    // Also set the seed in Lua state
    lua_pushinteger(L, seed);
    lua_setglobal(L, "__ENGINE_SEED");
}

} // namespace klab
