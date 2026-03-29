#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace klab {

struct Plane {
    glm::vec3 normal;
    float distance;

    Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) {
        float length = glm::length(n);
        normal = n / length;
        distance = d / length;
    }

    float getDistance(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

struct Frustum {
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;
};

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 1.6f, 5.0f));

    void update(float yaw, float pitch);
    void setPosition(glm::vec3 position) { m_position = position; }
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float fov, float aspect) const;

    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }

    void updateFrustum(float fov, float aspect, float nearP = 0.1f, float farP = 1000.0f);
    bool isInsideFrustum(const glm::vec3& center, float radius) const;
    bool isBoxInsideFrustum(const glm::vec3& min, const glm::vec3& max) const;

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    Frustum m_frustum;

    void updateVectors();
};

} // namespace klab
