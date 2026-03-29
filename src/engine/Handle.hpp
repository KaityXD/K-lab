#pragma once
#include <cstdint>
#include <memory>

namespace klab {

template<typename T>
class Handle {
public:
    Handle() : m_id(0), m_generation(0) {}
    Handle(uint32_t id, uint32_t generation) : m_id(id), m_generation(generation) {}

    bool isValid() const { return m_id != 0; }
    uint32_t getId() const { return m_id; }
    uint32_t getGeneration() const { return m_generation; }

    bool operator==(const Handle& other) const {
        return m_id == other.m_id && m_generation == other.m_generation;
    }
    bool operator!=(const Handle& other) const {
        return !(*this == other);
    }

private:
    uint32_t m_id;
    uint32_t m_generation;
};

} // namespace klab
