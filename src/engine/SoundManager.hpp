#pragma once
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "miniaudio.h"

namespace klab {

class SoundManager {
public:
    static SoundManager& getInstance();
    ~SoundManager();
    
    void playHit(float pitchOffset = 0.0f);
    void playHitAt(const glm::vec3& position, float pitchOffset = 0.0f);
    void playMiss();
    void playSound(const std::string& path, float pitchOffset = 0.0f);
    void playSoundAt(const std::string& path, const glm::vec3& position, float pitchOffset = 0.0f);

    void updateListener(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up = glm::vec3(0, 1, 0));
    
    void setHitSound(const std::string& path);
    void setMissSound(const std::string& path);
    
    void setHitSoundEnabled(bool enabled) { m_hitEnabled = enabled; }
    void setMissSoundEnabled(bool enabled) { m_missEnabled = enabled; }
    void setSpatializationEnabled(bool enabled) { m_spatializationEnabled = enabled; }
    bool isHitEnabled() const { return m_hitEnabled; }
    bool isMissEnabled() const { return m_missEnabled; }
    bool isSpatializationEnabled() const { return m_spatializationEnabled; }

    void setMasterVolume(float volume);
    void setHitVolume(float volume);
    void setMissVolume(float volume);
    float getMasterVolume() const { return m_masterVolume; }
    float getHitVolume() const { return m_hitVolume; }
    float getMissVolume() const { return m_missVolume; }

private:
    SoundManager();
    
    ma_engine m_engine;
    static const int HIT_POOL_SIZE = 16;
    ma_sound m_hitPool[HIT_POOL_SIZE];
    int m_nextHitIdx = 0;
    bool m_poolInitialized = false;

    static const int GENERIC_POOL_SIZE = 16;
    struct GenericSound {
        ma_sound sound;
        std::string currentPath;
        bool loaded = false;
    } m_genericPool[GENERIC_POOL_SIZE];
    int m_nextGenericIdx = 0;
    
    ma_sound m_missSoundObj;
    bool m_missLoaded = false;
    bool m_initialized = false;
    
    std::string m_hitSound;
    std::string m_missSound;
    bool m_hitEnabled = true;
    bool m_missEnabled = true;
    bool m_spatializationEnabled = true;

    float m_masterVolume = 1.0f;
    float m_hitVolume = 0.5f;
    float m_missVolume = 0.3f;
};

} // namespace klab
