#pragma once
#include <glm/glm.hpp>
#include <string>

namespace klab {

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

struct PhysicsComponent {
    glm::vec3 velocity = glm::vec3(0.0f);
    float gravity = 0.0f;
    bool active = true;

    void update(Transform& transform, float deltaTime) {
        if (!active) return;
        velocity.y -= gravity * deltaTime;
        transform.position += velocity * deltaTime;
    }
};

struct HealthComponent {
    float health = 1.0f;
    float maxHealth = 1.0f;
    float hitFlashTimer = 0.0f;
    bool hasHealthBar = false;
    bool isInvincible = false;

    void takeDamage(float amount) {
        if (isInvincible) {
            hitFlashTimer = 0.1f;
            return;
        }
        health -= amount;
        if (health < 0) health = 0;
        hitFlashTimer = 0.1f;
    }

    bool isDead() const { return health <= 0; }
    
    void update(float deltaTime) {
        if (hitFlashTimer > 0) {
            hitFlashTimer -= deltaTime;
        }
    }
};

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;

    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

    std::string getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

protected:
    Transform m_transform;
    std::string m_name;
};

} // namespace klab
