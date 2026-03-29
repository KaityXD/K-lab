#include "ScenarioBrowser.hpp"
#include "imgui.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <set>
#include "Settings.hpp"
#include "../game/ScoreManager.hpp"
#include "../engine/SoundManager.hpp"

namespace fs = std::filesystem;

namespace klab {

std::vector<ScenarioMetadata> ScenarioBrowser::m_scenarios;
std::vector<std::string> ScenarioBrowser::m_categories;
int ScenarioBrowser::m_selectedIdx = -1;
char ScenarioBrowser::m_searchQuery[128] = "";
bool ScenarioBrowser::m_initialized = false;

void ScenarioBrowser::refreshMetadata(AppState& state) {
    m_scenarios.clear();
    std::set<std::string> uniqueTags;
    uniqueTags.insert("All");
    
    if (fs::exists("scenarios") && fs::is_directory("scenarios")) {
        for (const auto& entry : fs::directory_iterator("scenarios")) {
            if (entry.path().extension() == ".lua") {
                ScenarioMetadata meta;
                meta.path = entry.path().string();
                
                meta.name = LuaManager::getGlobalString(meta.path, "scenarioName");
                if (meta.name.empty()) {
                    meta.name = entry.path().filename().stem().string();
                }

                meta.description = LuaManager::getGlobalString(meta.path, "description");
                meta.tags = LuaManager::getGlobalStrings(meta.path, "tags");
                for (auto& tag : meta.tags) {
                    if (!tag.empty()) {
                        std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
                        tag[0] = std::toupper(tag[0]);
                        uniqueTags.insert(tag);
                    }
                }

                auto const& highScores = ScoreManager::getInstance().getHighScores();
                std::string lookupName = meta.name;
                if (!highScores.count(lookupName)) {
                    lookupName = entry.path().filename().string();
                }

                if (highScores.count(lookupName)) {
                    meta.bestScore = highScores.at(lookupName).score;
                    meta.avgAccuracy = highScores.at(lookupName).accuracy;
                    meta.scoreTimeline = highScores.at(lookupName).scoreTimeline;
                }

                if (state.favorites.count(meta.path)) {
                    meta.isFavorite = true;
                }

                m_scenarios.push_back(meta);
            }
        }
    }
    
    m_categories.clear();
    m_categories.push_back("All");
    for (const auto& tag : uniqueTags) {
        if (tag != "All") m_categories.push_back(tag);
    }

    std::sort(m_scenarios.begin(), m_scenarios.end(), [](const auto& a, const auto& b) {
        if (a.isFavorite != b.isFavorite) return a.isFavorite;
        return a.name < b.name;
    });

    m_initialized = true;
}

void ScenarioBrowser::render(AppState& state, Scenario& scenario, LuaManager& lua) {
    ImGui::SetNextWindowSize(ImVec2(800 * state.hudScale, 600 * state.hudScale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scenario Browser", nullptr)) {
        renderContent(state, scenario, lua);
    }
    ImGui::End();
}

void ScenarioBrowser::renderContent(AppState& state, Scenario& scenario, LuaManager& lua) {
    if (!m_initialized) {
        refreshMetadata(state);
    }

    // Header: Search & Filter Chips
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##search", "Search scenarios...", m_searchQuery, sizeof(m_searchQuery))) {
        // Search handled in draw list
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();
    
    for (int i = 0; i < (int)m_categories.size(); i++) {
        bool selected = (state.tempCategoryFilter == i);
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        
        if (ImGui::Button(m_categories[i].c_str(), ImVec2(0, 0))) {
            state.tempCategoryFilter = i;
        }
        
        if (selected) ImGui::PopStyleColor();
        if (i < (int)m_categories.size() - 1) ImGui::SameLine();
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 120 * state.hudScale);
    ImGui::Checkbox("Favorites", &state.tempFavFilter);
    
    ImGui::PopStyleVar();
    ImGui::Separator();
    ImGui::Spacing();

    // Body
    if (ImGui::BeginChild("BrowserBody", ImVec2(0, 0), false)) {
        float listWidth = ImGui::GetContentRegionAvail().x * 0.45f;
        
        // Left Column: List
        ImGui::BeginChild("ListPart", ImVec2(listWidth, 0), true);
        drawCenterArea(state, scenario, lua);
        ImGui::EndChild();
        
        ImGui::SameLine();

        // Right Column: Details
        ImGui::BeginChild("DetailsPart", ImVec2(0, 0), false);
        drawRightPanel(state, scenario, lua);
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void ScenarioBrowser::drawLeftSidebar(AppState& state) {
    // Obsolete in minimal layout, logic moved to renderContent
}

void ScenarioBrowser::drawCenterArea(AppState& state, Scenario& scenario, LuaManager& lua) {
    std::string search = m_searchQuery;
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

    if (state.tempCategoryFilter >= (int)m_categories.size()) state.tempCategoryFilter = 0;
    std::string currentCat = m_categories[state.tempCategoryFilter];
    std::transform(currentCat.begin(), currentCat.end(), currentCat.begin(), ::tolower);

    for (int i = 0; i < (int)m_scenarios.size(); i++) {
        const auto& meta = m_scenarios[i];
        
        if (state.tempFavFilter && !meta.isFavorite) continue;

        if (state.tempCategoryFilter != 0) {
            bool found = false;
            for (const auto& tag : meta.tags) {
                std::string lTag = tag;
                std::transform(lTag.begin(), lTag.end(), lTag.begin(), ::tolower);
                if (lTag == currentCat) {
                    found = true;
                    break;
                }
            }
            if (!found) continue;
        }

        std::string lowerName = meta.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (!search.empty() && lowerName.find(search) == std::string::npos) {
            continue;
        }

        ImGui::PushID(i);
        bool isSelected = (m_selectedIdx == i);
        
        std::string label = (meta.isFavorite ? "* " : "  ") + meta.name;
        if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            m_selectedIdx = i;
            if (ImGui::IsMouseDoubleClicked(0)) {
                // Play immediately on double click?
            }
        }
        
        if (isSelected) {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40 * state.hudScale);
            ImGui::TextDisabled("%d", meta.bestScore);
        }

        ImGui::PopID();
    }
}

void ScenarioBrowser::drawRightPanel(AppState& state, Scenario& scenario, LuaManager& lua) {
    if (m_selectedIdx < 0 || m_selectedIdx >= (int)m_scenarios.size()) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 100, avail.y * 0.5f - 20));
        ImGui::TextDisabled("Select a scenario to start");
        return;
    }
    
    const auto& meta = m_scenarios[m_selectedIdx];
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font or bold if available
    ImGui::Text("%s", meta.name.c_str());
    ImGui::PopFont();
    
    if (meta.isFavorite) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "[FAVORITE]");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (!meta.description.empty()) {
        ImGui::TextWrapped("%s", meta.description.c_str());
        ImGui::Spacing();
    } else {
        ImGui::TextDisabled("No description available.");
        ImGui::Spacing();
    }
    
    ImGui::Columns(2, "StatCols", false);
    ImGui::TextDisabled("Best Score");
    ImGui::Text("%d", meta.bestScore);
    ImGui::NextColumn();
    ImGui::TextDisabled("Avg Accuracy");
    ImGui::Text("%.1f%%", meta.avgAccuracy);
    ImGui::Columns(1);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (!meta.scoreTimeline.empty()) {
        std::vector<float> timeline;
        for (int s : meta.scoreTimeline) timeline.push_back((float)s);
        ImGui::TextDisabled("Recent Progress");
        ImGui::SetNextItemWidth(-1);
        ImGui::PlotLines("##Progress", timeline.data(), (int)timeline.size(), 0, nullptr, 0.0f, (float)meta.bestScore * 1.1f, ImVec2(0, 60 * state.hudScale));
    }
    
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 100 * state.hudScale);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if (ImGui::Button("PLAY", ImVec2(-1, 40 * state.hudScale))) {
        state.currentScenario = -1;
        for(size_t i=0; i<state.scenarioFiles.size(); ++i) {
            if (state.scenarioFiles[i] == meta.path) {
                state.currentScenario = (int)i;
                break;
            }
        }
        
        scenario.reset();
        lua.loadFile(meta.path);
        lua.onStart();
        state.hitStreak = 0;
        state.displayedScore = 0;
        state.menuOpen = false;
        SettingsManager::saveConfig(state);
    }
    
    if (ImGui::Button(meta.isFavorite ? "Unfavorite" : "Favorite", ImVec2(-1, 30 * state.hudScale))) {
        if (meta.isFavorite) state.favorites.erase(meta.path);
        else state.favorites.insert(meta.path);
        
        m_scenarios[m_selectedIdx].isFavorite = !meta.isFavorite;
        SettingsManager::saveConfig(state);
        
        // Resort list to keep favorites at top
        std::sort(m_scenarios.begin(), m_scenarios.end(), [](const auto& a, const auto& b) {
            if (a.isFavorite != b.isFavorite) return a.isFavorite;
            return a.name < b.name;
        });
        
        // Re-find selection index
        for(int i=0; i<(int)m_scenarios.size(); ++i) {
            if (m_scenarios[i].path == meta.path) {
                m_selectedIdx = i;
                break;
            }
        }
    }
    ImGui::PopStyleVar();
}

} // namespace klab
