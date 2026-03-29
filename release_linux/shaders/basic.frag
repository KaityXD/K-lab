#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isFloor;
uniform bool simpleVisuals;

void main() {
    // Basic Grid for Floor
    vec3 baseColor = color;
    if (isFloor) {
        float grid = 0.0;
        float thickness = 0.02;
        if (fract(FragPos.x + 0.5) < thickness || fract(FragPos.z + 0.5) < thickness) {
            grid = 0.3;
        }
        baseColor = mix(color, color + vec3(grid), 0.5);
    }

    if (simpleVisuals) {
        FragColor = vec4(baseColor, 1.0);
        return;
    }

    // Simple Phong Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * vec3(1.0);
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
