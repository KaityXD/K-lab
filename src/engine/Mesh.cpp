#include "Mesh.hpp"
#include <cmath>
#include <vector>

namespace klab {

MeshData createSphere(int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float x, y, z, xy;                              
    float nx, ny, nz, lengthInv = 1.0f / 1.0f;    
    float s, t;                                     
    float sectorStep = 2 * glm::pi<float>() / sectors;
    float stackStep = glm::pi<float>() / stacks;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stacks; ++i) {
        stackAngle = glm::pi<float>() / 2 - i * stackStep;        
        xy = 0.5f * cosf(stackAngle);             
        z = 0.5f * sinf(stackAngle);              

        for(int j = 0; j <= sectors; ++j) {
            sectorAngle = j * sectorStep;           
            x = xy * cosf(sectorAngle);             
            y = xy * sinf(sectorAngle);             
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
            s = (float)j / sectors;
            t = (float)i / stacks;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    for(int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);     
        int k2 = k1 + sectors + 1;      
        for(int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if(i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if(i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    return {VAO, VBO, EBO, 0, (int)indices.size()};
}

MeshData createBean(int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float sectorStep = 2 * glm::pi<float>() / sectors;
    float stackStep = (glm::pi<float>() / 2) / (stacks / 2);
    
    float r = 0.5f;
    float h = 1.0f; 

    for(int i = 0; i <= stacks/2; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * stackStep;
        float xy = r * cosf(stackAngle);
        float y = r * sinf(stackAngle) + h/2.0f;

        for(int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float z = xy * sinf(sectorAngle);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
            glm::vec3 n = glm::normalize(glm::vec3(x, y - h/2.0f, z));
            vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
            vertices.push_back((float)j/sectors); vertices.push_back((float)i/stacks);
        }
    }

    for(int i = 0; i <= stacks/2; ++i) {
        float stackAngle = -i * stackStep;
        float xy = r * cosf(stackAngle);
        float y = r * sinf(stackAngle) - h/2.0f;

        for(int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float z = xy * sinf(sectorAngle);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
            glm::vec3 n = glm::normalize(glm::vec3(x, y + h/2.0f, z));
            vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
            vertices.push_back((float)j/sectors); vertices.push_back(0.5f + (float)i/stacks);
        }
    }

    int halfStacks = stacks / 2;
    for(int i = 0; i < halfStacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for(int j = 0; j < sectors; ++j, ++k1, ++k2) {
            indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
            indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
        }
    }
    int offset = (halfStacks + 1) * (sectors + 1);
    for(int i = 0; i < halfStacks; ++i) {
        int k1 = offset + i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for(int j = 0; j < sectors; ++j, ++k1, ++k2) {
            indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
            indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
        }
    }
    int topStart = halfStacks * (sectors + 1);
    int botStart = offset;
    for(int j = 0; j < sectors; ++j) {
        indices.push_back(topStart + j); indices.push_back(botStart + j); indices.push_back(topStart + j + 1);
        indices.push_back(topStart + j + 1); indices.push_back(botStart + j); indices.push_back(botStart + j + 1);
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    return {VAO, VBO, EBO, 0, (int)indices.size()};
}

MeshData createCone(int sectors) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float sectorStep = 2 * glm::pi<float>() / sectors;
    float r = 0.5f;
    float h = 1.0f;

    // Tip of the cone
    vertices.push_back(0); vertices.push_back(h/2.0f); vertices.push_back(0);
    vertices.push_back(0); vertices.push_back(1); vertices.push_back(0);
    vertices.push_back(0.5f); vertices.push_back(1);

    // Base vertices
    for(int i = 0; i <= sectors; ++i) {
        float angle = i * sectorStep;
        float x = r * cosf(angle);
        float z = r * sinf(angle);
        vertices.push_back(x); vertices.push_back(-h/2.0f); vertices.push_back(z);
        glm::vec3 n = glm::normalize(glm::vec3(x, r/h, z));
        vertices.push_back(n.x); vertices.push_back(n.y); vertices.push_back(n.z);
        vertices.push_back((float)i/sectors); vertices.push_back(0);
    }

    // Base center
    int baseCenterIdx = (int)vertices.size() / 8;
    vertices.push_back(0); vertices.push_back(-h/2.0f); vertices.push_back(0);
    vertices.push_back(0); vertices.push_back(-1); vertices.push_back(0);
    vertices.push_back(0.5f); vertices.push_back(0);

    // Side indices
    for(int i = 0; i < sectors; ++i) {
        indices.push_back(0);
        indices.push_back(i + 2);
        indices.push_back(i + 1);
    }

    // Base indices
    for(int i = 0; i < sectors; ++i) {
        indices.push_back(baseCenterIdx);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    return {VAO, VBO, EBO, 0, (int)indices.size()};
}

MeshData createQuad() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    return {VAO, VBO, EBO, 0, 6};
}

MeshData createBox() {
    float vertices[] = {
        // Positions          // Normals           // TexCoords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    return {VAO, VBO, EBO, 0, 36};
}

} // namespace klab
