#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace klab {

Camera::Camera(glm::vec3 position)
    : m_position(position), m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), m_yaw(-90.0f), m_pitch(0.0f) {
    updateVectors();
}

void Camera::update(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = pitch;

    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float fov, float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 1000.0f);
}

void Camera::updateFrustum(float fov, float aspect, float nearP, float farP) {
    const glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearP, farP);
    const glm::mat4 view = getViewMatrix();
    const glm::mat4 combo = proj * view;

    m_frustum.leftFace = Plane(
        glm::vec3(combo[0][3] + combo[0][0], combo[1][3] + combo[1][0], combo[2][3] + combo[2][0]),
        combo[3][3] + combo[3][0]
    );
    m_frustum.rightFace = Plane(
        glm::vec3(combo[0][3] - combo[0][0], combo[1][3] - combo[1][0], combo[2][3] - combo[2][0]),
        combo[3][3] - combo[3][0]
    );
    m_frustum.bottomFace = Plane(
        glm::vec3(combo[0][3] + combo[0][1], combo[1][3] + combo[1][1], combo[2][3] + combo[2][1]),
        combo[3][3] + combo[3][1]
    );
    m_frustum.topFace = Plane(
        glm::vec3(combo[0][3] - combo[0][1], combo[1][3] - combo[1][1], combo[2][3] - combo[2][1]),
        combo[3][3] - combo[3][1]
    );
    m_frustum.nearFace = Plane(
        glm::vec3(combo[0][3] + combo[0][2], combo[1][3] + combo[1][2], combo[2][3] + combo[2][2]),
        combo[3][3] + combo[3][2]
    );
    m_frustum.farFace = Plane(
        glm::vec3(combo[0][3] - combo[0][2], combo[1][3] - combo[1][2], combo[2][3] - combo[2][2]),
        combo[3][3] - combo[3][2]
    );
}

bool Camera::isInsideFrustum(const glm::vec3& center, float radius) const {
    if (m_frustum.leftFace.getDistance(center) < -radius) return false;
    if (m_frustum.rightFace.getDistance(center) < -radius) return false;
    if (m_frustum.topFace.getDistance(center) < -radius) return false;
    if (m_frustum.bottomFace.getDistance(center) < -radius) return false;
    if (m_frustum.nearFace.getDistance(center) < -radius) return false;
    if (m_frustum.farFace.getDistance(center) < -radius) return false;
    return true;
}

bool Camera::isBoxInsideFrustum(const glm::vec3& min, const glm::vec3& max) const {
    const Plane* planes[] = {&m_frustum.leftFace, &m_frustum.rightFace, &m_frustum.topFace, &m_frustum.bottomFace, &m_frustum.nearFace, &m_frustum.farFace};
    for(int i = 0; i < 6; ++i) {
        glm::vec3 p = min;
        if(planes[i]->normal.x >= 0) p.x = max.x;
        if(planes[i]->normal.y >= 0) p.y = max.y;
        if(planes[i]->normal.z >= 0) p.z = max.z;
        if(planes[i]->getDistance(p) < 0) return false;
    }
    return true;
}

void Camera::updateVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace klab

