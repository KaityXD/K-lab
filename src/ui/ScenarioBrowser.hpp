#pragma once

#include "../game/AppState.hpp"
#include "../game/Scenario.hpp"
#include "../scripting/LuaManager.hpp"
#include <string>
#include <vector>

namespace klab {

struct ScenarioMetadata {
    std::string name;
    std::string path;
    std::string description;
    std::vector<std::string> tags;
    int difficulty = 1;
    bool isFavorite = false;
    int playCount = 0;
    std::string lastPlayed;
    int bestScore = 0;
    float avgAccuracy = 0.0f;
    std::vector<int> scoreTimeline;
};

class ScenarioBrowser {
public:
    static void render(AppState& state, Scenario& scenario, LuaManager& lua);
    static void renderContent(AppState& state, Scenario& scenario, LuaManager& lua);

private:
    static void drawLeftSidebar(AppState& state);
    static void drawCenterArea(AppState& state, Scenario& scenario, LuaManager& lua);
    static void drawRightPanel(AppState& state, Scenario& scenario, LuaManager& lua);
    
    static void refreshMetadata(AppState& state);
    
    static std::vector<ScenarioMetadata> m_scenarios;
    static std::vector<std::string> m_categories;
    static int m_selectedIdx;
    static char m_searchQuery[128];
    static bool m_initialized;
};

} // namespace klab
