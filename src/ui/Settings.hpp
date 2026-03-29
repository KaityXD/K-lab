#pragma once

#include "../game/AppState.hpp"
#include "../game/Scenario.hpp"
#include "../scripting/LuaManager.hpp"
#include "../engine/Window.hpp"

namespace klab {

class SettingsManager {
public:
    static void refreshScenarios(AppState& state);
    static void refreshSounds(AppState& state);
    static void refreshTextures(AppState& state);
    static void loadCrosshairs(AppState& state);
    static void saveCrosshairs(const AppState& state);
    static void loadConfig(AppState& state);
    static void saveConfig(const AppState& state);
    static void saveCurrentScore(Scenario& scenario, const AppState& state);
    static void resetScenario(Scenario& scenario, LuaManager& lua, AppState& state);
};

} // namespace klab
