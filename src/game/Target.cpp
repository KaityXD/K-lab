#define GLM_ENABLE_EXPERIMENTAL
#include "Target.hpp"
#include <glm/gtx/intersect.hpp>
#include <algorithm>

namespace klab {

Target::Target(glm::vec3 position, float radius, TargetShape shape, glm::vec3 color)
    : m_radius(radius), m_shape(shape), m_active(true), m_color(color), m_timeActive(0.0f) {
    m_transform.position = position;
    m_transform.scale = glm::vec3(1.0f);
    
    if (shape == TargetShape::SPHERE) m_name = "Sphere";
    else if (shape == TargetShape::BEAN) m_name = "Bean";
    else if (shape == TargetShape::CONE) m_name = "Cone";
    else m_name = "Box";
}

void Target::update(float deltaTime) {
    if (!m_active) return;
    
    m_physics.update(m_transform, deltaTime);
    m_health.update(deltaTime);
    
    m_timeActive += deltaTime;
}

// Intersection logic remains the same, but uses m_transform.position and m_transform.scale
static float capIntersect(glm::vec3 ro, glm::vec3 rd, glm::vec3 pa, glm::vec3 pb, float r) {
    glm::vec3 ba = pb - pa;
    glm::vec3 oa = ro - pa;
    float baba = glm::dot(ba, ba);
    float bard = glm::dot(ba, rd);
    float baoa = glm::dot(ba, oa);
    float rdoa = glm::dot(rd, oa);
    float oaoa = glm::dot(oa, oa);
    float a = baba - bard * bard;
    float b = baba * rdoa - baoa * bard;
    float c = baba * oaoa - baoa * baoa - r * r * baba;
    float h = b * b - a * c;
    if (h >= 0.0f) {
        float t = (-b - sqrt(h)) / a;
        float y = baoa + t * bard;
        if (y > 0.0f && y < baba) return t;
        glm::vec3 oc = (y <= 0.0f) ? oa : ro - pb;
        b = glm::dot(rd, oc);
        c = glm::dot(oc, oc) - r * r;
        h = b * b - c;
        if (h > 0.0f) return -b - sqrt(h);
    }
    return -1.0f;
}

static float coneIntersect(glm::vec3 ro, glm::vec3 rd, glm::vec3 pa, glm::vec3 pb, float ra) {
    glm::vec3 ba = pb - pa;
    glm::vec3 oa = ro - pa;
    float baba = glm::dot(ba, ba);
    float bard = glm::dot(ba, rd);
    float baoa = glm::dot(ba, oa);
    float r2 = ra * ra;

    float a = baba - bard * bard * (1.0f + r2 / baba);
    float b = baba * glm::dot(oa, rd) - baoa * bard * (1.0f + r2 / baba);
    float c = baba * glm::dot(oa, oa) - baoa * baoa * (1.0f + r2 / baba) - r2 * baba;

    float h = b * b - a * c;
    if (h < 0.0f) return -1.0f;
    float t = (-b - sqrt(h)) / a;

    float y = baoa + t * bard;
    if (y > 0.0f && y < baba) return t;

    float t_cap = (baba - baoa) / bard;
    if (t_cap > 0.0f) {
        glm::vec3 pos = ro + t_cap * rd;
        glm::vec3 diff = pos - pb;
        if (glm::dot(diff, diff) < r2) return t_cap;
    }
    return -1.0f;
}

static float intersectUnitAABB(const glm::vec3& ro, const glm::vec3& rd) {
    glm::vec3 tMin = (glm::vec3(-1.0f) - ro) / rd;
    glm::vec3 tMax = (glm::vec3(1.0f) - ro) / rd;
    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);
    float tNear = std::max(std::max(t1.x, t1.y), t1.z);
    float tFar = std::min(std::min(t2.x, t2.y), t2.z);
    if (tNear > tFar || tFar < 0.0f) return -1.0f;
    return tNear;
}

float Target::checkHit(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const {
    if (!m_active) return -1.0f;

    glm::vec3 localRO = (rayOrigin - m_transform.position) / (m_radius * m_transform.scale);
    glm::vec3 localRD = rayDir / (m_radius * m_transform.scale);
    
    float rayScale = glm::length(localRD);
    localRD /= rayScale;

    float t = -1.0f;
    switch (m_shape) {
        case TargetShape::SPHERE: {
            float distance;
            if (glm::intersectRaySphere(localRO, localRD, glm::vec3(0), 1.0f, distance)) {
                t = distance;
            }
            break;
        }
        case TargetShape::BEAN:
            t = capIntersect(localRO, localRD, glm::vec3(0, -1.0f, 0), glm::vec3(0, 1.0f, 0), 1.0f);
            break;
        case TargetShape::CONE:
            t = coneIntersect(localRO, localRD, glm::vec3(0, 1.0f, 0), glm::vec3(0, -1.0f, 0), 1.0f);
            break;
        case TargetShape::BOX:
            t = intersectUnitAABB(localRO, localRD);
            break;
    }

    if (t > 0.0f) return t / rayScale;
    return -1.0f;
}

float Target::intersectAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const {
    return checkHit(rayOrigin, rayDir);
}

} // namespace klab
