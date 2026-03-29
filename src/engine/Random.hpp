#pragma once
#include <random>
#include <stdint.h>

namespace klab {

class Random {
public:
    static Random& getInstance() {
        static Random instance;
        return instance;
    }

    // Set seed for deterministic randomness
    void setSeed(uint32_t seed) {
        m_seed = seed;
        m_generator.seed(seed);
    }

    uint32_t getSeed() const { return m_seed; }

    // Get random float in range [0, 1)
    float getFloat() {
        return m_realDist(m_generator);
    }

    // Get random float in range [min, max)
    float getFloat(float min, float max) {
        return min + (max - min) * getFloat();
    }

    // Get random int in range [min, max]
    int getInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(m_generator);
    }

    // Get random bool with probability
    bool getBool(float probability = 0.5f) {
        return getFloat() < probability;
    }

    // Gaussian/normal distribution
    float getGaussian(float mean, float stddev) {
        std::normal_distribution<float> dist(mean, stddev);
        return dist(m_generator);
    }

    // Poisson distribution (for event timing)
    int getPoisson(float lambda) {
        std::poisson_distribution<int> dist(lambda);
        return dist(m_generator);
    }

    // Get random element from vector
    template<typename T>
    const T& getRandomElement(const std::vector<T>& vec) {
        if (vec.empty()) {
            static T empty{};
            return empty;
        }
        return vec[getInt(0, vec.size() - 1)];
    }

    // Shuffle vector (for randomizing order)
    template<typename T>
    void shuffleVector(std::vector<T>& vec) {
        for (int i = vec.size() - 1; i > 0; --i) {
            int j = getInt(0, i);
            std::swap(vec[i], vec[j]);
        }
    }

private:
    Random() : m_seed(0), m_generator(std::random_device{}()), m_realDist(0.0f, 1.0f) {}
    
    uint32_t m_seed;
    std::mt19937 m_generator;
    std::uniform_real_distribution<float> m_realDist;
};

// Global convenience functions
inline float randomFloat() { return Random::getInstance().getFloat(); }
inline float randomFloat(float min, float max) { return Random::getInstance().getFloat(min, max); }
inline int randomInt(int min, int max) { return Random::getInstance().getInt(min, max); }
inline bool randomBool(float prob = 0.5f) { return Random::getInstance().getBool(prob); }
inline float randomGaussian(float mean, float stddev) { return Random::getInstance().getGaussian(mean, stddev); }
inline int randomPoisson(float lambda) { return Random::getInstance().getPoisson(lambda); }

} // namespace klab