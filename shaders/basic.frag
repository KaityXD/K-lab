#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isFloor;
uniform bool simpleVisuals;
uniform bool useTexture;
uniform bool useTiling;
uniform bool floorTiling;
uniform float wallTileScale;
uniform float floorTileScale;
uniform sampler2D tex;
uniform vec3 objectScale;

void main() {
    vec3 norm = normalize(Normal);
    
    // 0. Linearize input color
    vec3 baseColor = pow(color, vec3(2.2));
    
    // 1. World-Space UV Tiling (Prevent stretching)
    vec2 uv = TexCoords;
    
    bool currentTiling = isFloor ? floorTiling : useTiling;
    float currentScale = isFloor ? floorTileScale : wallTileScale;

    if (currentTiling) {
        vec3 absNorm = abs(norm);
        if (absNorm.x > 0.9) {
            uv.x *= objectScale.z;
            uv.y *= objectScale.y;
        } else if (absNorm.y > 0.9) {
            uv.x *= objectScale.x;
            uv.y *= objectScale.z;
        } else {
            uv.x *= objectScale.x;
            uv.y *= objectScale.y;
        }
        uv *= currentScale;
    }

    if (useTexture) {
        baseColor *= pow(texture(tex, uv).rgb, vec3(2.2));
    } else if (isFloor) {
        float grid = 0.0;
        float thickness = 0.02;
        if (fract(FragPos.x + 0.5) < thickness || fract(FragPos.z + 0.5) < thickness) {
            grid = 0.3;
        }
        baseColor = mix(baseColor, baseColor + vec3(grid), 0.5);
    }

    if (simpleVisuals) {
        if (useTexture) {
            baseColor *= pow(texture(tex, uv).rgb, vec3(2.2));
        }
        FragColor = vec4(pow(baseColor, vec3(1.0/2.2)), 1.0);
        return;
    }

    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // 2. Hemisphere Ambient
    float up = dot(norm, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
    vec3 ambient = mix(vec3(0.02, 0.02, 0.05), vec3(0.1, 0.1, 0.12), up);
    ambient += 0.02; // Global constant ambient base
    
    // 3. Diffuse (Lower intensity to prevent blowout)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.8, 0.8, 0.75); 
    
    // 4. Blinn-Phong Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;
    
    // 5. Subtle Rim Lighting
    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 4.0);
    vec3 rimLight = rim * vec3(0.15);

    vec3 result = (ambient + diffuse + specular + rimLight) * baseColor;
    
    // 6. Gamma Correction
    result = pow(result, vec3(1.0 / 2.2));
    
    FragColor = vec4(result, 1.0);
}
