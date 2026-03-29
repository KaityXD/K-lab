#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (std140) uniform GlobalMatrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform float outlineWidth;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 worldNormal = normalize(normalMatrix * aNormal);
    worldPos.xyz += worldNormal * outlineWidth;
    gl_Position = projection * view * worldPos;
}
