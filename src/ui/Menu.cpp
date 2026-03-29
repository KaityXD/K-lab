#include "Menu.hpp"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include "Settings.hpp"
#include "../game/ScoreManager.hpp"
#include "../engine/SoundManager.hpp"
#include "../engine/TextureManager.hpp"
#include "../engine/Input.hpp"

namespace fs = std::filesystem;

#include "ScenarioBrowser.hpp"

namespace klab {

void Menu::render(AppState& state, Scenario& scenario, LuaManager& lua, Window& window) {
    if (!state.menuOpen) return;
    
    ImGui::SetNextWindowSize(ImVec2(850 * state.hudScale, 600 * state.hudScale), ImGuiCond_FirstUseEver);
    ImGui::Begin("K-Lab Settings", &state.menuOpen);
    if (ImGui::BeginTabBar("MenuTabs")) {
        if (ImGui::BeginTabItem("General")) {
            renderGeneralTab(state, scenario, lua, window);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scenarios")) {
            ScenarioBrowser::renderContent(state, scenario, lua);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("History")) {
            renderHistoryTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sensitivity")) {
            renderSensitivityTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Crosshair")) {
            renderCrosshairTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Visuals")) {
            renderVisualsTab(state, window);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio")) {
            renderAudioTab(state);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("High Scores")) {
            renderHighScoresTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Menu::renderGeneralTab(AppState& state, Scenario& scenario, LuaManager& lua, Window& window) {
    if (scenario.getTimeRemaining() <= 0) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Session Complete!");
        ImGui::Text("Final Score: %d", scenario.getScore());
        ImGui::Text("Accuracy: %.1f%%", scenario.getAccuracy());
        ImGui::Text("KPS: %.2f", scenario.getKPS());
        ImGui::Text("Avg Reaction Time: %.0f ms", scenario.getAvgReactionTime() * 1000.0f);
        ImGui::Text("Avg Flick Speed: %.2f units/s", scenario.getAvgFlickSpeed());
        ImGui::Separator();
    }
    if (ImGui::Checkbox("Fullscreen", &state.fullscreen)) window.toggleFullscreen();
    ImGui::Checkbox("Simple Shading", &state.simpleShading);
    ImGui::Checkbox("Hit Particles", &state.showParticles);
    ImGui::Checkbox("Hitmarkers", &state.showHitmarkers);
    ImGui::Checkbox("Killfeed", &state.showKillfeed);
    
    ImGui::Separator();
    if (state.currentScenario < (int)state.scenarioDisplayNames.size()) {
        ImGui::Text("Current Scenario: %s", state.scenarioDisplayNames[state.currentScenario].c_str());
    }
    ImGui::InputInt("FPS Target (0=Uncapped)", &state.fpsTarget);
    ImGui::Separator();
    if (ImGui::Button("Resume")) { state.menuOpen = false; SettingsManager::saveConfig(state); }
    if (ImGui::Button("Reset Scenario (R)")) { 
        SettingsManager::resetScenario(scenario, lua, state);
        state.menuOpen = false;
    }
    if (ImGui::Button("Exit")) window.close();
}

void Menu::renderHistoryTab(AppState& state) {
    if (state.currentScenario < state.scenarioFiles.size()) {
        std::string sName = state.scenarioFiles[state.currentScenario];
        size_t lastSlash = sName.find_last_of("/\\");
        if (lastSlash != std::string::npos) sName = sName.substr(lastSlash + 1);
        
        auto history = ScoreManager::getInstance().getRunHistory(sName);
        
        if (!history.empty()) {
            std::vector<float> scores;
            for (const auto& run : history) scores.push_back((float)run.score);
            ImGui::Text("Performance Trend (Last %d runs)", (int)scores.size());
            ImGui::PlotLines("##Scores", scores.data(), (int)scores.size(), 0, NULL, 0.0f, FLT_MAX, ImVec2(0, 80));
            ImGui::Separator();
        }

        if (ImGui::BeginTable("HistoryTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
            ImGui::TableSetupColumn("Score");
            ImGui::TableSetupColumn("Acc%");
            ImGui::TableSetupColumn("RT (ms)");
            ImGui::TableSetupColumn("Flick");
            ImGui::TableSetupColumn("Date");
            ImGui::TableHeadersRow();
            
            for (int i = (int)history.size() - 1; i >= 0; --i) {
                const auto& run = history[i];
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%d", run.score);
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.1f%%", run.accuracy);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.0f", run.reactionTime * 1000.0f);
                ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f", run.flickSpeed);
                ImGui::TableSetColumnIndex(4); ImGui::Text("%s", run.date.c_str());
            }
            ImGui::EndTable();
        }
    } else {
        ImGui::Text("Select a scenario to view history.");
    }
}

void Menu::renderSensitivityTab(AppState& state) {
    const char* presets[] = { "Custom (cm/360)", "Source / Apex / CS", "Valorant", "Overwatch / COD / Quake", "Roblox" };
    
    if (ImGui::Combo("Sensitivity Preset", &state.currentSensPreset, presets, IM_ARRAYSIZE(presets))) {
        // Recalculate if preset changed
        if (state.currentSensPreset > 0) {
            float multiplier = 1.0f;
            if (state.currentSensPreset == 1) multiplier = 0.022f; // Source
            else if (state.currentSensPreset == 2) multiplier = 0.069969f; // Valorant
            else if (state.currentSensPreset == 3) multiplier = 0.0066f; // OW
            else if (state.currentSensPreset == 4) multiplier = 1.010789f; // Roblox
            
            state.cm360 = (360.0f * 2.54f) / (state.gameSensitivity * multiplier * state.dpi);
        }
    }

    if (state.currentSensPreset == 0) {
        ImGui::InputFloat("cm/360", &state.cm360);
    } else {
        if (ImGui::InputFloat("Game Sensitivity", &state.gameSensitivity)) {
            float multiplier = 1.0f;
            if (state.currentSensPreset == 1) multiplier = 0.022f; // Source
            else if (state.currentSensPreset == 2) multiplier = 0.069969f; // Valorant
            else if (state.currentSensPreset == 3) multiplier = 0.0066f; // OW
            else if (state.currentSensPreset == 4) multiplier = 1.010789f; // Roblox
            
            state.cm360 = (360.0f * 2.54f) / (state.gameSensitivity * multiplier * state.dpi);
        }
        ImGui::Text("Calculated cm/360: %.2f", state.cm360);
    }

    if (ImGui::InputFloat("DPI", &state.dpi)) {
        if (state.currentSensPreset > 0) {
            float multiplier = 1.0f;
            if (state.currentSensPreset == 1) multiplier = 0.022f; // Source
            else if (state.currentSensPreset == 2) multiplier = 0.069969f; // Valorant
            else if (state.currentSensPreset == 3) multiplier = 0.0066f; // OW
            else if (state.currentSensPreset == 4) multiplier = 1.010789f; // Roblox
            state.cm360 = (360.0f * 2.54f) / (state.gameSensitivity * multiplier * state.dpi);
        }
    }

    ImGui::SliderFloat("FOV", &state.fov, 60.0f, 140.0f);
}

void Menu::renderCrosshairTab(AppState& state) {
    ImGui::SliderFloat("Thickness", &state.crosshair.thickness, 0.5f, 10.0f);
    ImGui::SliderFloat("Length", &state.crosshair.length, 0.0f, 100.0f);
    ImGui::SliderFloat("Gap", &state.crosshair.gap, -20.0f, 50.0f);
    
    ImGui::Separator();
    ImGui::Checkbox("Dot", &state.crosshair.dot);
    if (state.crosshair.dot) ImGui::SliderFloat("Dot Size", &state.crosshair.dotSize, 0.5f, 20.0f);
    
    ImGui::Separator();
    ImGui::Checkbox("Circle", &state.crosshair.circle);
    if (state.crosshair.circle) ImGui::SliderFloat("Circle Radius", &state.crosshair.circleRadius, 0.5f, 100.0f);

    ImGui::Separator();
    ImGui::Checkbox("T-Shape", &state.crosshair.tShape);
    ImGui::Text("Visible Lines:");
    ImGui::Checkbox("Top", &state.crosshair.drawTop); ImGui::SameLine();
    ImGui::Checkbox("Right", &state.crosshair.drawRight); ImGui::SameLine();
    ImGui::Checkbox("Bottom", &state.crosshair.drawBottom); ImGui::SameLine();
    ImGui::Checkbox("Left", &state.crosshair.drawLeft);

    ImGui::Separator();
    ImGui::Checkbox("Outline", &state.crosshair.outline);
    if (state.crosshair.outline) ImGui::SliderFloat("Outline Thickness", &state.crosshair.outlineThickness, 0.5f, 5.0f);
    ImGui::ColorEdit4("Outline Color", state.crosshair.outlineColor);
    
    ImGui::Separator();
    ImGui::ColorEdit4("Crosshair Color", state.crosshair.color);
    ImGui::Separator();
    ImGui::Text("User Presets");
    ImGui::InputText("Preset Name", state.presetNameBuf, 64);
    if (ImGui::Button("Save Current as Preset")) {
        state.xhairPresets[std::string(state.presetNameBuf)] = state.crosshair;
        SettingsManager::saveCrosshairs(state);
    }
    
    if (!state.xhairPresets.empty()) {
        static std::string selectedPreset = "";
        if (ImGui::BeginCombo("Load Preset", selectedPreset.c_str())) {
            for (auto const& [name, s] : state.xhairPresets) {
                if (ImGui::Selectable(name.c_str())) {
                    state.crosshair = s;
                    selectedPreset = name;
                    strncpy(state.presetNameBuf, name.c_str(), 64);
                }
            }
            ImGui::EndCombo();
        }
        if (!selectedPreset.empty() && ImGui::Button("Delete Selected Preset")) {
            state.xhairPresets.erase(selectedPreset);
            SettingsManager::saveCrosshairs(state);
            selectedPreset = "";
        }
    }
    ImGui::Separator();
    ImGui::Text("Quick Presets");
    if (ImGui::Button("Small Dot")) {
        state.crosshair = CrosshairSettings();
        state.crosshair.dot = true; state.crosshair.dotSize = 1.5f; state.crosshair.thickness = 0; state.crosshair.length = 0; state.crosshair.gap = 0;
        state.crosshair.outline = true; state.crosshair.outlineThickness = 1.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Classic Four")) {
        state.crosshair = CrosshairSettings();
        state.crosshair.dot = false; state.crosshair.thickness = 2.0f; state.crosshair.length = 10.0f; state.crosshair.gap = 5.0f;
        state.crosshair.outline = true; state.crosshair.outlineThickness = 1.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Thick Green")) {
        state.crosshair = CrosshairSettings();
        state.crosshair.dot = false; state.crosshair.thickness = 4.0f; state.crosshair.length = 12.0f; state.crosshair.gap = 2.0f;
        state.crosshair.color[0] = 0; state.crosshair.color[1] = 1; state.crosshair.color[2] = 0; state.crosshair.color[3] = 1;
        state.crosshair.outline = true; state.crosshair.outlineThickness = 1.5f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Circle Pro")) {
        state.crosshair = CrosshairSettings();
        state.crosshair.dot = true; state.crosshair.dotSize = 1.0f;
        state.crosshair.circle = true; state.crosshair.circleRadius = 8.0f; state.crosshair.thickness = 1.5f;
        state.crosshair.length = 0; state.crosshair.gap = 0;
        state.crosshair.outline = true; state.crosshair.outlineThickness = 1.0f;
    }
}

void Menu::renderVisualsTab(AppState& state, Window& window) {
    ImGui::Checkbox("Show FPS", &state.showFPS);
    ImGui::ColorEdit3("Sky Color", state.skyColor);
    ImGui::ColorEdit3("Floor Color", state.floorColor);
    ImGui::ColorEdit3("Target Color", state.targetColor);
    ImGui::Separator();
    ImGui::Text("Target Visuals");
    ImGui::Checkbox("Target Outline", &state.targetOutline);
    ImGui::Checkbox("Target Flash on Hit", &state.targetFlash);
    if (state.targetOutline) {
        ImGui::SliderFloat("Outline Width", &state.targetOutlineWidth, 0.01f, 0.2f);
        ImGui::ColorEdit3("Outline Color", state.targetOutlineColor);
    }
    ImGui::Separator();
    ImGui::Text("Wall Texture Settings");
    ImGui::Checkbox("Wall Tiling", &state.useTiling);
    ImGui::SliderFloat("Wall Tile Scale", &state.wallTileScale, 0.01f, 10.0f);
    if (ImGui::Combo("Wall Texture", &state.currentWallTextureIdx, state.textureCStrs.data(), (int)state.textureCStrs.size())) {
        if (state.currentWallTextureIdx == 0) {
            glDeleteTextures(1, &state.wallTexture);
            state.wallTexture = generateCheckerboardTexture();
            state.currentWallTexturePath = "Default";
        } else {
            glDeleteTextures(1, &state.wallTexture);
            state.wallTexture = loadTexture(state.textureFiles[state.currentWallTextureIdx].c_str());
            state.currentWallTexturePath = state.textureFiles[state.currentWallTextureIdx];
        }
    }
    ImGui::Separator();
    ImGui::Text("Floor Texture Settings");
    ImGui::Checkbox("Floor Tiling", &state.floorTiling);
    ImGui::SliderFloat("Floor Tile Scale", &state.floorTileScale, 0.01f, 10.0f);
    if (ImGui::Button("Refresh Textures Folder")) SettingsManager::refreshTextures(state);
    ImGui::TextWrapped("Put your .png/.jpg files in assets/textures/ and click 'Refresh'.");

    ImGui::Separator();
    ImGui::Text("HUD Customization");
    ImGui::SliderFloat("HUD Scale", &state.hudScale, 0.5f, 2.0f);
    ImGui::ColorEdit4("HUD Text Color", state.hudColor);
    ImGui::Checkbox("Show Score", &state.hudShowScore);
    ImGui::Checkbox("Show Accuracy", &state.hudShowAccuracy);
    ImGui::Checkbox("Show KPS", &state.hudShowKPS);
    ImGui::Checkbox("Show Time", &state.hudShowTime);
    ImGui::Checkbox("Show Killfeed", &state.hudShowKillfeed);

    ImGui::Separator();
    ImGui::Text("Health Bar Customization");
    ImGui::Checkbox("Show Health Bars", &state.showHealthBars);
    if (state.showHealthBars) {
        ImGui::SliderFloat("Width", &state.healthBarWidth, 0.1f, 3.0f);
        ImGui::SliderFloat("Height", &state.healthBarHeight, 0.01f, 0.5f);
        ImGui::SliderFloat("Vertical Offset", &state.healthBarOffset, -1.0f, 2.0f);
        ImGui::ColorEdit3("Color High", state.healthBarColorHigh);
        ImGui::ColorEdit3("Color Low", state.healthBarColorLow);
        ImGui::ColorEdit3("Background Color", state.healthBarColorBg);
    }
}

void Menu::renderAudioTab(AppState& state) {
    if (ImGui::Button("Refresh Sounds Folder")) SettingsManager::refreshSounds(state);
    ImGui::Separator();
    
    if (ImGui::SliderFloat("Master Volume", &state.masterVol, 0.0f, 1.0f)) {
        SoundManager::getInstance().setMasterVolume(state.masterVol);
    }
    if (ImGui::SliderFloat("Hit Volume", &state.hitVol, 0.0f, 1.0f)) {
        SoundManager::getInstance().setHitVolume(state.hitVol);
    }
    if (ImGui::SliderFloat("Miss Volume", &state.missVol, 0.0f, 1.0f)) {
        SoundManager::getInstance().setMissVolume(state.missVol);
    }

    ImGui::Separator();
    if (ImGui::Checkbox("Hit Sound Enabled", &state.hitEnabled)) {
        SoundManager::getInstance().setHitSoundEnabled(state.hitEnabled);
    }
    bool spatial = SoundManager::getInstance().isSpatializationEnabled();
    if (ImGui::Checkbox("Enable 3D Spatialization", &spatial)) {
        SoundManager::getInstance().setSpatializationEnabled(spatial);
    }
    if (ImGui::Combo("Hit Sound", &state.currentHitSound, state.soundCStrs.data(), (int)state.soundCStrs.size())) {
        if (state.currentHitSound == 0) state.hitSound = "/usr/share/sounds/alsa/Front_Center.wav";
        else state.hitSound = state.soundFiles[state.currentHitSound];
        SoundManager::getInstance().setHitSound(state.hitSound);
    }
    if (ImGui::Button("Test Hit Sound")) SoundManager::getInstance().playHit();
    ImGui::SliderFloat("Hit Pitch Rand (Cents)", &state.hitPitchRandomization, 0.0f, 200.0f);
    ImGui::Separator();
    if (ImGui::Checkbox("Miss Sound Enabled", &state.missEnabled)) {
        SoundManager::getInstance().setMissSoundEnabled(state.missEnabled);
    }
    if (ImGui::Combo("Miss Sound", &state.currentMissSound, state.soundCStrs.data(), (int)state.soundCStrs.size())) {
        if (state.currentMissSound == 0) state.missSound = "/usr/share/sounds/alsa/Noise.wav";
        else state.missSound = state.soundFiles[state.currentMissSound];
        SoundManager::getInstance().setMissSound(state.missSound);
    }
    if (ImGui::Button("Test Miss Sound")) SoundManager::getInstance().playMiss();
    ImGui::Separator();
    ImGui::TextWrapped("Put your .wav files in assets/sounds/ and click 'Refresh'.");
}

void Menu::renderHighScoresTab() {
    if (ImGui::BeginTable("ScoresTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Scenario");
        ImGui::TableSetupColumn("Score");
        ImGui::TableSetupColumn("Acc%");
        ImGui::TableSetupColumn("Date");
        ImGui::TableHeadersRow();
        auto scores = ScoreManager::getInstance().getHighScores();
        for (auto const& [name, hs] : scores) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", name.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", hs.score);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f%%", hs.accuracy);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%s", hs.date.c_str());
        }
        ImGui::EndTable();
    }
}

void Menu::renderHUD(AppState& state, Scenario& scenario, Window& window, float fpsDisplay) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(state.hudScale);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(state.hudColor[0], state.hudColor[1], state.hudColor[2], state.hudColor[3]));
    if (state.showFPS) {
        ImGui::Text("FPS: %.0f", fpsDisplay);
        ImGui::Text("Mouse Latency: %.2f ms (%.0f Hz)", Input::getMouseLatency(), Input::getMousePollingRate());
        ImGui::Text("Input Stale: %.2f ms", Input::getTimeSinceLastSample());
    }
    if (state.hudShowScore) ImGui::Text("Score: %d", (int)state.displayedScore);
    if (state.hudShowAccuracy) ImGui::Text("Accuracy: %.1f%%", scenario.getAccuracy());
    if (state.hudShowKPS) ImGui::Text("KPS: %.2f", scenario.getKPS());
    if (state.hudShowTime) ImGui::Text("Time: %.1f", scenario.getTimeRemaining());
    
    if (state.hudShowKillfeed && !state.killfeed.empty()) {
        ImGui::Separator();
        for (const auto& entry : state.killfeed) {
            ImGui::TextColored(ImVec4(1, 1, 0, entry.life / 1.5f), "%s", entry.text.c_str());
        }
    }
    ImGui::PopStyleColor();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(window.getWidth() / 2.0f - 100.0f * state.hudScale, window.getHeight() - 100.0f * state.hudScale));
    ImGui::Begin("Progress HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(state.hudScale);
    if (state.currentScenario < (int)state.scenarioFiles.size()) {
        std::string sName = state.scenarioFiles[state.currentScenario];
        size_t lastSlash = sName.find_last_of("/\\");
        if (lastSlash != std::string::npos) sName = sName.substr(lastSlash + 1);
        
        auto highScores = ScoreManager::getInstance().getHighScores();
        if (highScores.count(sName)) {
            const auto& hs = highScores.at(sName);
            float progress = hs.score > 0 ? (float)state.displayedScore / hs.score : 0.0f;
            
            float elapsed = scenario.getInitialTime() - scenario.getTimeRemaining();
            float bestRunProgress = 0.0f;
            float bestAtThisTimeSmoothed = 0.0f;
            if (!hs.scoreTimeline.empty() && hs.score > 0) {
                int idx1 = (int)std::floor(elapsed);
                int idx2 = idx1 + 1;
                float t = elapsed - (float)idx1;
                
                float val1 = (float)hs.scoreTimeline[std::min(idx1, (int)hs.scoreTimeline.size() - 1)];
                float val2 = (idx2 < (int)hs.scoreTimeline.size()) ? (float)hs.scoreTimeline[idx2] : val1;
                
                bestAtThisTimeSmoothed = val1 + (val2 - val1) * t;
                bestRunProgress = bestAtThisTimeSmoothed / hs.score;
            }

            ImVec4 barColor = ImVec4(0.2f, 0.8f, 0.2f, 0.8f); 
            if ((float)state.displayedScore < bestAtThisTimeSmoothed) {
                barColor = ImVec4(0.8f, 0.2f, 0.2f, 0.8f); 
            }

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
            ImGui::ProgressBar(std::min(progress, 1.0f), ImVec2(200, 15), "");
            ImGui::PopStyleColor();
            
            if (bestRunProgress > 0) {
                ImVec2 pos = ImGui::GetItemRectMin();
                ImVec2 size = ImGui::GetItemRectSize();
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                float markerX = pos.x + size.x * std::min(bestRunProgress, 1.0f);
                drawList->AddLine(ImVec2(markerX, pos.y - 2), ImVec2(markerX, pos.y + size.y + 2), ImColor(255, 255, 255, 255), 2.5f * state.hudScale);
            }
        }
    }
    ImGui::End();
}

} // namespace klab
