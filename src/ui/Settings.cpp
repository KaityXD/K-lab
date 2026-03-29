#include "Settings.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../game/ScoreManager.hpp"
#include "../engine/SoundManager.hpp"
#include "../scripting/LuaManager.hpp"

namespace fs = std::filesystem;

namespace klab {

void SettingsManager::refreshScenarios(AppState& state) {
    state.scenarioFiles.clear();
    state.scenarioDisplayNames.clear();
    state.scenarioCStrs.clear();
    if (fs::exists("scenarios") && fs::is_directory("scenarios")) {
        for (const auto& entry : fs::directory_iterator("scenarios")) {
            if (entry.path().extension() == ".lua") {
                std::string path = entry.path().string();
                state.scenarioFiles.push_back(path);
                
                std::string friendlyName = LuaManager::getGlobalString(path, "scenarioName");
                if (friendlyName.empty()) {
                    friendlyName = entry.path().filename().string();
                }
                state.scenarioDisplayNames.push_back(friendlyName);
            }
        }
    }
    for (const auto& s : state.scenarioDisplayNames) {
        state.scenarioCStrs.push_back(s.c_str());
    }
}

void SettingsManager::refreshSounds(AppState& state) {
    state.soundFiles.clear();
    state.soundCStrs.clear();
    state.soundFiles.push_back("Default");
    if (fs::exists("assets/sounds") && fs::is_directory("assets/sounds")) {
        for (const auto& entry : fs::directory_iterator("assets/sounds")) {
            if (entry.path().extension() == ".wav") {
                state.soundFiles.push_back(entry.path().string());
            }
        }
    }
    for (const auto& s : state.soundFiles) {
        state.soundCStrs.push_back(s.c_str());
    }
}

void SettingsManager::refreshTextures(AppState& state) {
    state.textureFiles.clear();
    state.textureCStrs.clear();
    state.textureFiles.push_back("Default");
    if (fs::exists("assets/textures") && fs::is_directory("assets/textures")) {
        for (const auto& entry : fs::directory_iterator("assets/textures")) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
                state.textureFiles.push_back(entry.path().string());
            }
        }
    }
    for (const auto& t : state.textureFiles) {
        state.textureCStrs.push_back(t.c_str());
    }
}

void SettingsManager::loadCrosshairs(AppState& state) {
    state.xhairPresets.clear();
    std::ifstream f("crosshairs.cfg");
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            std::stringstream ss(line);
            std::string name; ss >> name;
            if (name.empty()) continue;
            std::replace(name.begin(), name.end(), '_', ' ');
            CrosshairSettings s;
            if (!(ss >> s.thickness >> s.length >> s.gap)) continue;
            
            int dot, outline, circle = 0, tShape = 0, dt = 1, dr = 1, db = 1, dl = 1;
            float dotSize = 2.0f, circleRadius = 10.0f, outThick = 1.0f;
            
            if (ss >> dot >> dotSize >> outline >> outThick >> circle >> circleRadius >> tShape >> dt >> dr >> db >> dl) {
                s.dot = (dot == 1); s.dotSize = dotSize;
                s.outline = (outline == 1); s.outlineThickness = outThick;
                s.circle = (circle == 1); s.circleRadius = circleRadius;
                s.tShape = (tShape == 1);
                s.drawTop = (dt == 1); s.drawRight = (dr == 1); s.drawBottom = (db == 1); s.drawLeft = (dl == 1);
            } else {
                ss.clear();
                ss.str(line);
                std::string dummy; ss >> dummy >> s.thickness >> s.length >> s.gap; 
                if (ss >> dot >> outline >> s.outlineThickness) {
                    s.dot = (dot == 1); s.outline = (outline == 1);
                }
            }
            ss >> s.color[0] >> s.color[1] >> s.color[2] >> s.color[3];
            ss >> s.outlineColor[0] >> s.outlineColor[1] >> s.outlineColor[2] >> s.outlineColor[3];
            state.xhairPresets[name] = s;
        }
    }
}

void SettingsManager::saveCrosshairs(const AppState& state) {
    std::ofstream f("crosshairs.cfg");
    for (auto const& [name, s] : state.xhairPresets) {
        std::string n = name;
        std::replace(n.begin(), n.end(), ' ', '_');
        f << n << " " << s.thickness << " " << s.length << " " << s.gap << " ";
        f << (s.dot ? 1 : 0) << " " << s.dotSize << " " << (s.outline ? 1 : 0) << " " << s.outlineThickness << " ";
        f << (s.circle ? 1 : 0) << " " << s.circleRadius << " " << (s.tShape ? 1 : 0) << " " 
          << (s.drawTop ? 1 : 0) << " " << (s.drawRight ? 1 : 0) << " " << (s.drawBottom ? 1 : 0) << " " << (s.drawLeft ? 1 : 0) << " ";
        f << s.color[0] << " " << s.color[1] << " " << s.color[2] << " " << s.color[3] << " ";
        f << s.outlineColor[0] << " " << s.outlineColor[1] << " " << s.outlineColor[2] << " " << s.outlineColor[3] << "\n";
    }
}

void SettingsManager::loadConfig(AppState& state) {
    std::ifstream f("config.cfg");
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            std::stringstream ss(line);
            std::string key; ss >> key;
            if (key == "cm360") ss >> state.cm360;
            else if (key == "dpi") ss >> state.dpi;
            else if (key == "fov") ss >> state.fov;
            else if (key == "sensPreset") ss >> state.currentSensPreset;
            else if (key == "gameSens") ss >> state.gameSensitivity;
            else if (key == "fpsTarget") ss >> state.fpsTarget;
            else if (key == "hitSound") ss >> state.hitSound;
            else if (key == "missSound") ss >> state.missSound;
            else if (key == "hitEnabled") { int e; ss >> e; state.hitEnabled = (e == 1); }
            else if (key == "missEnabled") { int e; ss >> e; state.missEnabled = (e == 1); }
            else if (key == "spatialization") { 
                int s; ss >> s; 
                SoundManager::getInstance().setSpatializationEnabled(s == 1); 
            }
            else if (key == "showFPS") ss >> state.showFPS;
            else if (key == "showParticles") { int s; ss >> s; state.showParticles = (s == 1); }
            else if (key == "showHitmarkers") { int s; ss >> s; state.showHitmarkers = (s == 1); }
            else if (key == "showKillfeed") { int s; ss >> s; state.showKillfeed = (s == 1); }
            else if (key == "fullscreen") ss >> state.fullscreen;
            else if (key == "simpleShading") { int s; ss >> s; state.simpleShading = (s == 1); }
            else if (key == "masterVol") ss >> state.masterVol;
            else if (key == "hitVol") ss >> state.hitVol;
            else if (key == "missVol") ss >> state.missVol;
            else if (key == "hitPitchRand") ss >> state.hitPitchRandomization;
            else if (key == "targetOutline") { int o; ss >> o; state.targetOutline = (o == 1); }
            else if (key == "targetFlash") { int f; ss >> f; state.targetFlash = (f == 1); }
            else if (key == "showHealthBars") { int s; ss >> s; state.showHealthBars = (s == 1); }
            else if (key == "hbWidth") ss >> state.healthBarWidth;
            else if (key == "hbHeight") ss >> state.healthBarHeight;
            else if (key == "hbOffset") ss >> state.healthBarOffset;
            else if (key == "hbColorLow") ss >> state.healthBarColorLow[0] >> state.healthBarColorLow[1] >> state.healthBarColorLow[2];
            else if (key == "hbColorHigh") ss >> state.healthBarColorHigh[0] >> state.healthBarColorHigh[1] >> state.healthBarColorHigh[2];
            else if (key == "hbColorBg") ss >> state.healthBarColorBg[0] >> state.healthBarColorBg[1] >> state.healthBarColorBg[2];
            else if (key == "hudScale") ss >> state.hudScale;
            else if (key == "hudColor") ss >> state.hudColor[0] >> state.hudColor[1] >> state.hudColor[2] >> state.hudColor[3];
            else if (key == "hudShowScore") { int s; ss >> s; state.hudShowScore = (s == 1); }
            else if (key == "hudShowAccuracy") { int s; ss >> s; state.hudShowAccuracy = (s == 1); }
            else if (key == "hudShowKPS") { int s; ss >> s; state.hudShowKPS = (s == 1); }
            else if (key == "hudShowTime") { int s; ss >> s; state.hudShowTime = (s == 1); }
            else if (key == "hudShowKillfeed") { int s; ss >> s; state.hudShowKillfeed = (s == 1); }
            else if (key == "targetOutColor") ss >> state.targetOutlineColor[0] >> state.targetOutlineColor[1] >> state.targetOutlineColor[2];
            else if (key == "targetOutWidth") ss >> state.targetOutlineWidth;
            else if (key == "wallTexturePath") ss >> state.currentWallTexturePath;
            else if (key == "useTiling") { int t; ss >> t; state.useTiling = (t == 1); }
            else if (key == "floorTiling") { int t; ss >> t; state.floorTiling = (t == 1); }
            else if (key == "wallTileScale") ss >> state.wallTileScale;
            else if (key == "floorTileScale") ss >> state.floorTileScale;
            else if (key == "scenarioName") {
                std::string sName; ss >> sName;
                for (size_t i = 0; i < state.scenarioFiles.size(); ++i) {
                    if (state.scenarioFiles[i] == sName) {
                        state.currentScenario = (int)i;
                        break;
                    }
                }
            }
            else if (key == "favorite") {
                std::string fPath; ss >> fPath;
                state.favorites.insert(fPath);
            }
            else if (key == "skyColor") ss >> state.skyColor[0] >> state.skyColor[1] >> state.skyColor[2];
            else if (key == "floorColor") ss >> state.floorColor[0] >> state.floorColor[1] >> state.floorColor[2];
            else if (key == "targetColor") ss >> state.targetColor[0] >> state.targetColor[1] >> state.targetColor[2];
            else if (key == "thick") ss >> state.crosshair.thickness;
            else if (key == "len") ss >> state.crosshair.length;
            else if (key == "gap") ss >> state.crosshair.gap;
            else if (key == "dot") { int d; ss >> d; state.crosshair.dot = (d == 1); }
            else if (key == "dotSize") ss >> state.crosshair.dotSize;
            else if (key == "circle") { int c; ss >> c; state.crosshair.circle = (c == 1); }
            else if (key == "circleRadius") ss >> state.crosshair.circleRadius;
            else if (key == "tShape") { int t; ss >> t; state.crosshair.tShape = (t == 1); }
            else if (key == "drawTop") { int t; ss >> t; state.crosshair.drawTop = (t == 1); }
            else if (key == "drawRight") { int r; ss >> r; state.crosshair.drawRight = (r == 1); }
            else if (key == "drawBottom") { int b; ss >> b; state.crosshair.drawBottom = (b == 1); }
            else if (key == "drawLeft") { int l; ss >> l; state.crosshair.drawLeft = (l == 1); }
            else if (key == "outline") { int o; ss >> o; state.crosshair.outline = (o == 1); }
            else if (key == "outThick") ss >> state.crosshair.outlineThickness;
            else if (key == "xColor") ss >> state.crosshair.color[0] >> state.crosshair.color[1] >> state.crosshair.color[2] >> state.crosshair.color[3];
            else if (key == "outColor") ss >> state.crosshair.outlineColor[0] >> state.crosshair.outlineColor[1] >> state.crosshair.outlineColor[2] >> state.crosshair.outlineColor[3];
        }
    }
}

void SettingsManager::saveConfig(const AppState& state) {
    std::ofstream f("config.cfg");
    f << "cm360 " << state.cm360 << "\n";
    f << "dpi " << state.dpi << "\n";
    f << "fov " << state.fov << "\n";
    f << "sensPreset " << state.currentSensPreset << "\n";
    f << "gameSens " << state.gameSensitivity << "\n";
    f << "fpsTarget " << state.fpsTarget << "\n";
    f << "hitSound " << state.hitSound << "\n";
    f << "missSound " << state.missSound << "\n";
    f << "hitEnabled " << (state.hitEnabled ? 1 : 0) << "\n";
    f << "missEnabled " << (state.missEnabled ? 1 : 0) << "\n";
    f << "spatialization " << (SoundManager::getInstance().isSpatializationEnabled() ? 1 : 0) << "\n";
    f << "showFPS " << (state.showFPS ? 1 : 0) << "\n";
    f << "showParticles " << (state.showParticles ? 1 : 0) << "\n";
    f << "showHitmarkers " << (state.showHitmarkers ? 1 : 0) << "\n";
    f << "showKillfeed " << (state.showKillfeed ? 1 : 0) << "\n";
    f << "fullscreen " << (state.fullscreen ? 1 : 0) << "\n";
    f << "simpleShading " << (state.simpleShading ? 1 : 0) << "\n";
    f << "masterVol " << state.masterVol << "\n";
    f << "hitVol " << state.hitVol << "\n";
    f << "missVol " << state.missVol << "\n";
    f << "hitPitchRand " << state.hitPitchRandomization << "\n";
    f << "targetOutline " << (state.targetOutline ? 1 : 0) << "\n";
    f << "targetFlash " << (state.targetFlash ? 1 : 0) << "\n";
    f << "showHealthBars " << (state.showHealthBars ? 1 : 0) << "\n";
    f << "hbWidth " << state.healthBarWidth << "\n";
    f << "hbHeight " << state.healthBarHeight << "\n";
    f << "hbOffset " << state.healthBarOffset << "\n";
    f << "hbColorLow " << state.healthBarColorLow[0] << " " << state.healthBarColorLow[1] << " " << state.healthBarColorLow[2] << "\n";
    f << "hbColorHigh " << state.healthBarColorHigh[0] << " " << state.healthBarColorHigh[1] << " " << state.healthBarColorHigh[2] << "\n";
    f << "hbColorBg " << state.healthBarColorBg[0] << " " << state.healthBarColorBg[1] << " " << state.healthBarColorBg[2] << "\n";
    f << "hudScale " << state.hudScale << "\n";
    f << "hudColor " << state.hudColor[0] << " " << state.hudColor[1] << " " << state.hudColor[2] << " " << state.hudColor[3] << "\n";
    f << "hudShowScore " << (state.hudShowScore ? 1 : 0) << "\n";
    f << "hudShowAccuracy " << (state.hudShowAccuracy ? 1 : 0) << "\n";
    f << "hudShowKPS " << (state.hudShowKPS ? 1 : 0) << "\n";
    f << "hudShowTime " << (state.hudShowTime ? 1 : 0) << "\n";
    f << "hudShowKillfeed " << (state.hudShowKillfeed ? 1 : 0) << "\n";
    f << "targetOutColor " << state.targetOutlineColor[0] << " " << state.targetOutlineColor[1] << " " << state.targetOutlineColor[2] << "\n";
    f << "targetOutWidth " << state.targetOutlineWidth << "\n";
    f << "wallTexturePath " << state.currentWallTexturePath << "\n";
    f << "useTiling " << (state.useTiling ? 1 : 0) << "\n";
    f << "floorTiling " << (state.floorTiling ? 1 : 0) << "\n";
    f << "wallTileScale " << state.wallTileScale << "\n";
    f << "floorTileScale " << state.floorTileScale << "\n";
    if (state.currentScenario < (int)state.scenarioFiles.size()) f << "scenarioName " << state.scenarioFiles[state.currentScenario] << "\n";
    f << "skyColor " << state.skyColor[0] << " " << state.skyColor[1] << " " << state.skyColor[2] << "\n";
    f << "floorColor " << state.floorColor[0] << " " << state.floorColor[1] << " " << state.floorColor[2] << "\n";
    f << "targetColor " << state.targetColor[0] << " " << state.targetColor[1] << " " << state.targetColor[2] << "\n";
    f << "thick " << state.crosshair.thickness << "\n";
    f << "len " << state.crosshair.length << "\n";
    f << "gap " << state.crosshair.gap << "\n";
    f << "dot " << (state.crosshair.dot ? 1 : 0) << "\n";
    f << "dotSize " << state.crosshair.dotSize << "\n";
    f << "circle " << (state.crosshair.circle ? 1 : 0) << "\n";
    f << "circleRadius " << state.crosshair.circleRadius << "\n";
    f << "tShape " << (state.crosshair.tShape ? 1 : 0) << "\n";
    f << "drawTop " << (state.crosshair.drawTop ? 1 : 0) << "\n";
    f << "drawRight " << (state.crosshair.drawRight ? 1 : 0) << "\n";
    f << "drawBottom " << (state.crosshair.drawBottom ? 1 : 0) << "\n";
    f << "drawLeft " << (state.crosshair.drawLeft ? 1 : 0) << "\n";
    f << "outline " << (state.crosshair.outline ? 1 : 0) << "\n";
    f << "outThick " << state.crosshair.outlineThickness << "\n";
    f << "xColor " << state.crosshair.color[0] << " " << state.crosshair.color[1] << " " << state.crosshair.color[2] << " " << state.crosshair.color[3] << "\n";
    f << "outColor " << state.crosshair.outlineColor[0] << " " << state.crosshair.outlineColor[1] << " " << state.crosshair.outlineColor[2] << " " << state.crosshair.outlineColor[3] << "\n";
    for (const auto& fav : state.favorites) {
        f << "favorite " << fav << "\n";
    }
}

void SettingsManager::saveCurrentScore(Scenario& scenario, const AppState& state) {
    if (!scenario.hasBeenSaved() && scenario.getTotalShots() > 0 && state.currentScenario < (int)state.scenarioFiles.size()) {
        std::string sName = state.scenarioFiles[state.currentScenario];
        size_t lastSlash = sName.find_last_of("/\\");
        if (lastSlash != std::string::npos) sName = sName.substr(lastSlash + 1);
        ScoreManager::getInstance().updateHighScore(sName, scenario.getScore(), scenario.getAccuracy(), scenario.getAvgReactionTime(), scenario.getAvgFlickSpeed(), scenario.getScoreTimeline());
        scenario.setHasBeenSaved(true);
    }
}

void SettingsManager::resetScenario(Scenario& scenario, LuaManager& lua, AppState& state) {
    scenario.reset();
    lua.onStart();
    state.hitStreak = 0;
    state.displayedScore = 0;
}

} // namespace klab
