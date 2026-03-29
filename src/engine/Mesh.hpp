#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace klab {

struct MeshData {
    unsigned int VAO, VBO, EBO;
    unsigned int VAO_instanced;
    int indexCount;
};

MeshData createSphere(int sectors, int stacks);
MeshData createBean(int sectors, int stacks);
MeshData createCone(int sectors);
MeshData createQuad();
MeshData createBox();

} // namespace klab
