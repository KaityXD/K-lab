#pragma once
#include "Entity.hpp"
#include <glm/glm.hpp>
#include <string>

namespace klab {

enum class TargetShape {
    BOX,
    SPHERE,
    BEAN,
    CONE
};

class Target : public Entity {
public:
    Target(glm::vec3 position, float radius = 0.5f, TargetShape shape = TargetShape::SPHERE, glm::vec3 color = glm::vec3(1,0,0));

    float checkHit(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;
    void update(float deltaTime);
    
    // Legacy getters/setters redirected to components
    glm::vec3 getPosition() const { return m_transform.position; }
    void setPosition(glm::vec3 pos) { m_transform.position = pos; }
    glm::vec3 getScale() const { return m_transform.scale; }
    void setScale(glm::vec3 scale) { m_transform.scale = scale; }

    float getRadius() const { return m_radius; }
    void setRadius(float r) { m_radius = r; }
    TargetShape getShape() const { return m_shape; }
    void setShape(TargetShape shape) { m_shape = shape; }
    glm::vec3 getColor() const { return m_color; }
    void setColor(glm::vec3 color) { m_color = color; }
    
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    void setVelocity(glm::vec3 vel) { m_physics.velocity = vel; }
    glm::vec3 getVelocity() const { return m_physics.velocity; }
    void setGravity(float g) { m_physics.gravity = g; }
    float getGravity() const { return m_physics.gravity; }

    bool isExpired() const { return m_lifetime > 0 && m_timeActive >= m_lifetime; }
    float getTimeActive() const { return m_timeActive; }
    void setLifetime(float seconds) { m_lifetime = seconds; }

    void setHealth(float hp) { m_health.health = hp; m_health.maxHealth = hp; }
    float getHealth() const { return m_health.health; }
    float getMaxHealth() const { return m_health.maxHealth; }
    bool hasHealthBar() const { return m_health.hasHealthBar; }
    void setHasHealthBar(bool b) { m_health.hasHealthBar = b; }
    void setInvincible(bool b) { m_health.isInvincible = b; }
    bool isInvincible() const { return m_health.isInvincible; }
    void takeDamage(float amount) { m_health.takeDamage(amount); }
    bool isDead() const { return m_health.isDead(); }
    float getHitFlashTimer() const { return m_health.hitFlashTimer; }

private:
    PhysicsComponent m_physics;
    HealthComponent m_health;

    glm::vec3 m_color;
    float m_radius;
    float m_lifetime = 0.0f; 
    float m_timeActive = 0.0f;
    TargetShape m_shape;
    bool m_active;

    float intersectAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;
};

} // namespace klab
