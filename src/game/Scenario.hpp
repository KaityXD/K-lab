#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Target.hpp"

namespace klab {

struct Wall {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    glm::vec3 rotation; // Euler angles in degrees
};

class Scenario {
public:
    Scenario();
    
    void spawnTarget(glm::vec3 pos, float radius = 0.5f, TargetShape shape = TargetShape::SPHERE, glm::vec3 color = glm::vec3(1,0,0), float health = 100.0f, const std::string& name = "");
    void spawnWall(glm::vec3 pos, glm::vec3 size, glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3 rotation = glm::vec3(0));
    void clearTargets();
    void clearWalls();
    void update(float deltaTime);
    bool checkHits(const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool isAutoShot = false);
    glm::vec3 getLastHitPos() const { return m_lastHitPos; }
    std::string getLastHitName() const { return m_lastHitName; }
    bool wasLastHitKill() const { return m_lastHitWasKill; }

    void setAutoShoot(bool autoShoot) { m_autoShoot = autoShoot; }
    bool isAutoShoot() const { return m_autoShoot; }
    void setIsAuto(bool isAuto) { m_isAuto = isAuto; }
    bool getIsAuto() const { return m_isAuto; }
    void setTargetsPersistent(bool persistent) { m_targetsPersistent = persistent; }
    
    void setFireRate(float rpm) { m_fireRate = rpm; }
    bool canShoot() const { return m_fireTimer <= 0; }
    void recordShot() { if (m_fireRate > 0) m_fireTimer = 60.0f / m_fireRate; }

    bool hasBeenSaved() const { return m_hasBeenSaved; }
    void setHasBeenSaved(bool saved) { m_hasBeenSaved = saved; }

    const std::vector<std::unique_ptr<Target>>& getTargets() const { return m_targets; }
    const std::vector<Wall>& getWalls() const { return m_walls; }
    int getTargetCount() const {
        int count = 0;
        for (const auto& t : m_targets) if (t->isActive()) count++;
        return count;
    }
    int getScore() const { return m_score; }
    void setScore(int score) { m_score = score; }
    const std::vector<int>& getScoreTimeline() const { return m_scoreTimeline; }
    int getHits() const { return m_hits; }
    void setHits(int hits) { m_hits = hits; }
    int getMisses() const { return m_misses; }
    void setMisses(int misses) { m_misses = misses; }
    int getTotalShots() const { return m_totalShots; }
    void setTotalShots(int shots) { m_totalShots = shots; }
    float getAccuracy() const { return m_totalShots > 0 ? (float)m_hits / m_totalShots * 100.0f : 0.0f; }
    float getKPS() const { float elapsed = m_initialTime - m_timeRemaining; return elapsed > 0 ? (float)m_hits / elapsed : 0.0f; }
    float getAvgReactionTime() const { return m_hitsWithReactionTime > 0 ? m_totalReactionTime / m_hitsWithReactionTime : 0.0f; }
    float getAvgFlickSpeed() const { return m_flickCount > 0 ? m_totalFlickSpeed / m_flickCount : 0.0f; }
    void setTimeRemaining(float seconds) { m_timeRemaining = seconds; m_initialTime = seconds; }
    float getTimeRemaining() const { return m_timeRemaining; }
    float getInitialTime() const { return m_initialTime; }

    void reset();

    private:
    std::vector<std::unique_ptr<Target>> m_targets;
    std::vector<Wall> m_walls;
    int m_score;
    int m_hits;
    int m_misses;
    int m_totalShots;
    float m_timeRemaining;
    float m_initialTime = 60.0f;

    float m_totalReactionTime = 0.0f;
    int m_hitsWithReactionTime = 0;
    float m_totalFlickSpeed = 0.0f;
    int m_flickCount = 0;
    double m_lastHitTime = 0.0;

    double m_lastScoreTime = 0;
    std::vector<int> m_scoreTimeline;
    float m_lastTimelineRecordTime = 0;
    bool m_autoShoot = false;
    bool m_isAuto = false;
    bool m_targetsPersistent = false;
    bool m_hasBeenSaved = false;
    float m_fireRate = 600.0f;
    float m_fireTimer = 0.0f;
    glm::vec3 m_lastHitPos = glm::vec3(0);
    std::string m_lastHitName;
    bool m_lastHitWasKill = false;
};

} // namespace klab
