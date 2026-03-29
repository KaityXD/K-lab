#include "Scenario.hpp"
#include <algorithm>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace klab {

Scenario::Scenario() : m_score(0), m_hits(0), m_misses(0), m_timeRemaining(60.0f), m_lastScoreTime(0), m_fireTimer(0.0f) {}

void Scenario::spawnTarget(glm::vec3 pos, float radius, TargetShape shape, glm::vec3 color, float health, const std::string& name) {
    auto target = std::make_unique<Target>(pos, radius, shape, color);
    target->setHealth(health);
    if (!name.empty()) target->setName(name);
    m_targets.push_back(std::move(target));
}

void Scenario::spawnWall(glm::vec3 pos, glm::vec3 size, glm::vec3 color, glm::vec3 rotation) {
    m_walls.push_back({pos, size, color, rotation});
}

void Scenario::clearTargets() {
    m_targets.clear();
}

void Scenario::clearWalls() {
    m_walls.clear();
}

void Scenario::update(float deltaTime) {
    m_timeRemaining -= deltaTime;
    if (m_timeRemaining < 0) m_timeRemaining = 0;

    // Record score timeline every second
    float elapsed = m_initialTime - m_timeRemaining;
    if (elapsed >= m_lastTimelineRecordTime + 1.0f) {
        m_scoreTimeline.push_back(m_score);
        m_lastTimelineRecordTime += 1.0f;
    }

    if (m_fireTimer > 0) {
        m_fireTimer -= deltaTime;
    }

    for (auto it = m_targets.begin(); it != m_targets.end(); ) {
        (*it)->update(deltaTime);
        if ((*it)->isExpired() || !(*it)->isActive()) {
            it = m_targets.erase(it);
        } else {
            ++it;
        }
    }
}

bool Scenario::checkHits(const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool isAutoShot) {
    m_totalShots++;
    bool hitFound = false;
    double currentTime = glfwGetTime();
    m_lastHitWasKill = false;

    // First check if we hit any wall that blocks the shot
    float minWallT = 1e10f;
    bool wallHit = false;
    for (const auto& wall : m_walls) {
        // Transform ray to local space of the wall
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, wall.position);
        model = glm::rotate(model, glm::radians(wall.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(wall.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(wall.rotation.z), glm::vec3(0, 0, 1));
        
        glm::mat4 invModel = glm::inverse(model);
        glm::vec3 localRO = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 localRD = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));
        
        // Ray-AABB in local space
        glm::vec3 halfExtent = wall.size * 0.5f;
        glm::vec3 min = -halfExtent;
        glm::vec3 max = halfExtent;
        
        float t1 = (min.x - localRO.x) / localRD.x;
        float t2 = (max.x - localRO.x) / localRD.x;
        float t3 = (min.y - localRO.y) / localRD.y;
        float t4 = (max.y - localRO.y) / localRD.y;
        float t5 = (min.z - localRO.z) / localRD.z;
        float t6 = (max.z - localRO.z) / localRD.z;

        float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        if (tmax >= 0 && tmin <= tmax) {
            if (tmin < minWallT) {
                minWallT = tmin;
                wallHit = true;
            }
        }
    }

    for (auto& target : m_targets) {
        if (target->isActive()) {
            float targetT = target->checkHit(rayOrigin, rayDir);
            if (targetT > 0.0f) {
                // Check if wall is in front of the target
                if (wallHit && minWallT < targetT) {
                    continue; // Wall is in front
                }

                // Damage the target
                target->takeDamage(100.0f); // Default shot damage
                
                bool isKill = target->isDead();
                m_lastHitWasKill = isKill;
                if (isKill && !m_targetsPersistent) {
                    target->setActive(false);
                }

                // Still count every hit for score
                m_hits++;
                m_score += 10;
                
                // Metrics - Only if it's NOT an auto-shot from a hitstreak
                if (!isAutoShot) {
                    float dt = (float)(currentTime - m_lastHitTime);
                    
                    // Track "Reaction Time" on manual click
                    if (dt > 0.010f) {
                        m_totalReactionTime += dt;
                        m_hitsWithReactionTime++;

                        float dist = glm::distance(m_lastHitPos, target->getPosition());
                        m_totalFlickSpeed += dist / dt;
                        m_flickCount++;
                    }
                }
                
                // ALWAYS update last hit info so the NEXT manual flick/reaction has a valid start point
                m_lastHitTime = currentTime;
                m_lastHitPos = target->getPosition();
                m_lastHitName = target->getName();
                
                hitFound = true;
                break;
            }
        }
    }

    if (!hitFound) {
        m_misses++;
    }
    return hitFound;
}

void Scenario::reset() {
    m_targets.clear();
    m_walls.clear();
    m_score = 0;
    m_hits = 0;
    m_misses = 0;
    m_totalShots = 0;
    m_initialTime = 60.0f; // Reset to default 60s
    m_timeRemaining = m_initialTime;
    m_lastScoreTime = 0;
    m_fireTimer = 0.0f;
    m_autoShoot = false;
    m_isAuto = false;
    m_targetsPersistent = false;
    m_hasBeenSaved = false;
    m_fireRate = 600.0f;
    
    // Initialize starting state for metrics
    m_lastHitPos = glm::vec3(0, 2, -10); 
    m_lastHitTime = glfwGetTime();
    
    m_totalReactionTime = 0.0f;
    m_hitsWithReactionTime = 0;
    m_totalFlickSpeed = 0.0f;
    m_flickCount = 0;

    m_scoreTimeline.clear();
    m_scoreTimeline.push_back(0); // Initial score at t=0
    m_lastTimelineRecordTime = 0.0f;
}

} // namespace klab
