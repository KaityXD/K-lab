#pragma once

#include "../game/AppState.hpp"
#include "../game/Scenario.hpp"
#include "../scripting/LuaManager.hpp"
#include "../engine/Window.hpp"

namespace klab {

class Menu {
public:
    static void render(AppState& state, Scenario& scenario, LuaManager& lua, Window& window);
    static void renderHUD(AppState& state, Scenario& scenario, Window& window, float fpsDisplay);

private:
    static void renderGeneralTab(AppState& state, Scenario& scenario, LuaManager& lua, Window& window);
    static void renderHistoryTab(AppState& state);
    static void renderSensitivityTab(AppState& state);
    static void renderCrosshairTab(AppState& state);
    static void renderVisualsTab(AppState& state, Window& window);
    static void renderAudioTab(AppState& state);
    static void renderHighScoresTab();
};

} // namespace klab
