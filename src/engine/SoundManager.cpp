#define MINIAUDIO_IMPLEMENTATION
#include "SoundManager.hpp"
#include <iostream>
#include <cmath>

namespace klab {

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() {
    ma_result result;
    result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        m_initialized = false;
    } else {
        m_initialized = true;
    }

    // Initialize pool
    m_poolInitialized = false;

    // Default sounds
    setHitSound("/usr/share/sounds/alsa/Front_Center.wav");
    setMissSound("/usr/share/sounds/alsa/Noise.wav");
}

SoundManager::~SoundManager() {
    if (m_initialized) {
        if (m_poolInitialized) {
            for (int i = 0; i < HIT_POOL_SIZE; ++i) {
                ma_sound_uninit(&m_hitPool[i]);
            }
        }
        for (int i = 0; i < GENERIC_POOL_SIZE; ++i) {
            if (m_genericPool[i].loaded) {
                ma_sound_uninit(&m_genericPool[i].sound);
            }
        }
        if (m_missLoaded) ma_sound_uninit(&m_missSoundObj);
        ma_engine_uninit(&m_engine);
    }
}

void SoundManager::playHit(float pitchOffset) {
    if (!m_hitEnabled || !m_initialized || !m_poolInitialized) return;
    
    // Get next sound from pool
    ma_sound* sound = &m_hitPool[m_nextHitIdx];
    m_nextHitIdx = (m_nextHitIdx + 1) % HIT_POOL_SIZE;
    
    // Restart sound if playing
    ma_sound_seek_to_pcm_frame(sound, 0);
    
    // Set pitch: 2^(cents / 1200)
    float pitchMultiplier = std::pow(2.0f, pitchOffset / 1200.0f);
    ma_sound_set_pitch(sound, pitchMultiplier);
    ma_sound_set_volume(sound, m_hitVolume);
    
    // Disable spatialization for centered hit sound
    ma_sound_set_spatialization_enabled(sound, false);
    
    ma_sound_start(sound);
}

void SoundManager::playHitAt(const glm::vec3& position, float pitchOffset) {
    if (!m_hitEnabled || !m_initialized || !m_poolInitialized) return;
    
    ma_sound* sound = &m_hitPool[m_nextHitIdx];
    m_nextHitIdx = (m_nextHitIdx + 1) % HIT_POOL_SIZE;
    
    ma_sound_seek_to_pcm_frame(sound, 0);
    float pitchMultiplier = std::pow(2.0f, pitchOffset / 1200.0f);
    ma_sound_set_pitch(sound, pitchMultiplier);
    ma_sound_set_volume(sound, m_hitVolume);
    
    // Enable/disable spatialization and set position
    ma_sound_set_spatialization_enabled(sound, m_spatializationEnabled);
    if (m_spatializationEnabled) {
        ma_sound_set_position(sound, position.x, position.y, position.z);
        // Set rolloff model (Inverse is standard)
        ma_sound_set_attenuation_model(sound, ma_attenuation_model_inverse);
    }
    
    ma_sound_start(sound);
}

void SoundManager::playMiss() {
    if (m_missEnabled && m_initialized) {
        if (m_missLoaded) {
            ma_sound_seek_to_pcm_frame(&m_missSoundObj, 0);
            ma_sound_set_volume(&m_missSoundObj, m_missVolume);
            ma_sound_set_spatialization_enabled(&m_missSoundObj, false);
            ma_sound_start(&m_missSoundObj);
        } else {
            ma_engine_play_sound(&m_engine, m_missSound.c_str(), NULL);
        }
    }
}

void SoundManager::updateListener(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) {
    if (!m_initialized) return;
    // index 0 is the default listener
    ma_engine_listener_set_position(&m_engine, 0, position.x, position.y, position.z);
    ma_engine_listener_set_direction(&m_engine, 0, direction.x, direction.y, direction.z);
    ma_engine_listener_set_world_up(&m_engine, 0, up.x, up.y, up.z);
}

void SoundManager::setMasterVolume(float volume) {
    m_masterVolume = volume;
    if (m_initialized) ma_engine_set_volume(&m_engine, volume);
}

void SoundManager::setHitVolume(float volume) {
    m_hitVolume = volume;
    if (m_poolInitialized) {
        for (int i = 0; i < HIT_POOL_SIZE; ++i) {
            ma_sound_set_volume(&m_hitPool[i], volume);
        }
    }
}

void SoundManager::setMissVolume(float volume) {
    m_missVolume = volume;
    if (m_missLoaded) ma_sound_set_volume(&m_missSoundObj, volume);
}

void SoundManager::setHitSound(const std::string& path) {
    if (path.empty()) return;
    m_hitSound = path;
    if (m_initialized) {
        if (m_poolInitialized) {
            for (int i = 0; i < HIT_POOL_SIZE; ++i) {
                ma_sound_uninit(&m_hitPool[i]);
            }
            m_poolInitialized = false;
        }
        
        // Initialize pool with the new sound
        bool success = true;
        for (int i = 0; i < HIT_POOL_SIZE; ++i) {
            // Load the sound into memory (MA_SOUND_FLAG_DECODE) for low latency
            if (ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &m_hitPool[i]) != MA_SUCCESS) {
                success = false;
                break;
            }
        }
        m_poolInitialized = success;
    }
}

void SoundManager::setMissSound(const std::string& path) {
    if (path.empty()) return;
    m_missSound = path;
    if (m_initialized) {
        if (m_missLoaded) ma_sound_uninit(&m_missSoundObj);
        if (ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &m_missSoundObj) == MA_SUCCESS) {
            m_missLoaded = true;
        }
    }
}

void SoundManager::playSound(const std::string& path, float pitchOffset) {
    if (!m_initialized || path.empty()) return;
    
    GenericSound& gs = m_genericPool[m_nextGenericIdx];
    m_nextGenericIdx = (m_nextGenericIdx + 1) % GENERIC_POOL_SIZE;

    if (gs.loaded && gs.currentPath != path) {
        ma_sound_uninit(&gs.sound);
        gs.loaded = false;
    }

    if (!gs.loaded) {
        if (ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &gs.sound) == MA_SUCCESS) {
            gs.loaded = true;
            gs.currentPath = path;
        }
    }

    if (gs.loaded) {
        ma_sound_seek_to_pcm_frame(&gs.sound, 0);
        float pitchMultiplier = std::pow(2.0f, pitchOffset / 1200.0f);
        ma_sound_set_pitch(&gs.sound, pitchMultiplier);
        ma_sound_set_volume(&gs.sound, m_masterVolume);
        ma_sound_set_spatialization_enabled(&gs.sound, false);
        ma_sound_start(&gs.sound);
    }
}

void SoundManager::playSoundAt(const std::string& path, const glm::vec3& position, float pitchOffset) {
    if (!m_initialized || path.empty()) return;
    
    GenericSound& gs = m_genericPool[m_nextGenericIdx];
    m_nextGenericIdx = (m_nextGenericIdx + 1) % GENERIC_POOL_SIZE;

    if (gs.loaded && gs.currentPath != path) {
        ma_sound_uninit(&gs.sound);
        gs.loaded = false;
    }

    if (!gs.loaded) {
        if (ma_sound_init_from_file(&m_engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &gs.sound) == MA_SUCCESS) {
            gs.loaded = true;
            gs.currentPath = path;
        }
    }

    if (gs.loaded) {
        ma_sound_seek_to_pcm_frame(&gs.sound, 0);
        float pitchMultiplier = std::pow(2.0f, pitchOffset / 1200.0f);
        ma_sound_set_pitch(&gs.sound, pitchMultiplier);
        ma_sound_set_volume(&gs.sound, m_masterVolume);
        ma_sound_set_spatialization_enabled(&gs.sound, m_spatializationEnabled);
        if (m_spatializationEnabled) {
            ma_sound_set_position(&gs.sound, position.x, position.y, position.z);
            ma_sound_set_attenuation_model(&gs.sound, ma_attenuation_model_inverse);
        }
        ma_sound_start(&gs.sound);
    }
}

} // namespace klab
