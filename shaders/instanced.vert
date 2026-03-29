#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceModel;
layout (location = 7) in vec3 aInstanceColor;
layout (location = 8) in vec3 aInstanceObjectScale;

layout (std140) uniform GlobalMatrices {
    mat4 projection;
    mat4 view;
};

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 InstanceColor;
out vec3 InstanceObjectScale;

void main() {
    vec4 worldPos = aInstanceModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(aInstanceModel) * aNormal;
    TexCoords = aTexCoords;
    InstanceColor = aInstanceColor;
    InstanceObjectScale = aInstanceObjectScale;
    gl_Position = projection * view * worldPos;
}
