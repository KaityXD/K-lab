#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <glm/glm.hpp>
#include "../ui/Crosshair.hpp"
#include "../engine/Renderer.hpp"

namespace klab {

struct Hitmarker { 
    float life; 
    glm::vec3 pos; 
};

struct KillFeedEntry { 
    std::string text; 
    float life; 
};

struct AppState {
    static AppState* getInstance() { return s_instance; }
    static void setInstance(AppState* inst) { s_instance = inst; }

    // Input/Sensitivity
    float yaw = -90.0f;
    float pitch = 0.0f;
    float cm360 = 34.64f;
    float dpi = 800.0f;
    float fov = 103.0f;
    int fpsTarget = 0;

    // Sensitivity Presets
    int currentSensPreset = 0; // 0 = Custom/cm360, 1 = Source/Apex, 2 = Valorant, 3 = Overwatch/COD, 4 = Quake
    float gameSensitivity = 1.0f;

    // Audio
    std::string hitSound = "/usr/share/sounds/alsa/Front_Center.wav";
    std::string missSound = "/usr/share/sounds/alsa/Noise.wav";
    int currentHitSound = 0;
    int currentMissSound = 0;
    bool hitEnabled = true;
    bool missEnabled = true;
    float masterVol = 1.0f;
    float hitVol = 0.5f;
    float missVol = 0.3f;
    float hitPitchRandomization = 50.0f; // in cents

    // Visuals
    float floorColor[3] = { 0.2f, 0.2f, 0.2f };
    float targetColor[3] = { 1.0f, 0.0f, 0.0f };
    float skyColor[3] = { 0.1f, 0.1f, 0.1f };
    bool showFPS = true;
    bool fullscreen = false;
    bool simpleShading = false;
    bool showParticles = true;
    bool showHitmarkers = true;
    bool showKillfeed = true;
    bool targetOutline = true;
    bool targetFlash = true;
    float targetOutlineColor[3] = { 1.0f, 1.0f, 1.0f };
    float targetOutlineWidth = 0.05f;

    // Health Bar Customization
    bool showHealthBars = true;
    float healthBarWidth = 1.0f;
    float healthBarHeight = 0.12f;
    float healthBarOffset = 0.2f;
    float healthBarColorLow[3] = { 1.0f, 0.0f, 0.0f };
    float healthBarColorHigh[3] = { 0.0f, 1.0f, 0.0f };
    float healthBarColorBg[3] = { 0.15f, 0.15f, 0.15f };

    // HUD Customization
    float hudScale = 1.0f;
    float hudColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool hudShowScore = true;
    bool hudShowAccuracy = true;
    bool hudShowKPS = true;
    bool hudShowTime = true;
    bool hudShowKillfeed = true;

    // Textures
    int currentWallTextureIdx = 0;
    std::string currentWallTexturePath = "Default";
    unsigned int wallTexture = 0;
    bool useTiling = true;
    bool floorTiling = true;
    float wallTileScale = 1.0f;
    float floorTileScale = 0.5f;

    // Scenarios
    int currentScenario = 0;
    std::set<std::string> favorites;
    std::vector<std::string> scenarioFiles;
    std::vector<std::string> scenarioDisplayNames;
    std::vector<const char*> scenarioCStrs;

    // Assets Lists
    std::vector<std::string> soundFiles;
    std::vector<const char*> soundCStrs;
    std::vector<std::string> textureFiles;
    std::vector<const char*> textureCStrs;

    // Crosshair
    CrosshairSettings crosshair;
    std::map<std::string, CrosshairSettings> xhairPresets;
    char presetNameBuf[64] = "My Crosshair";

    // Game Logic
    int hitStreak = 0;
    float displayedScore = 0.0f;
    std::vector<Hitmarker> hitmarkers;
    std::vector<KillFeedEntry> killfeed;

    // Menu State
    bool menuOpen = true;
    bool wasMenuOpen = false;

    // Temporary filters for UI
    int tempCategoryFilter = 0;
    bool tempFavFilter = false;

private:
    static inline AppState* s_instance = nullptr;
};

} // namespace klab
