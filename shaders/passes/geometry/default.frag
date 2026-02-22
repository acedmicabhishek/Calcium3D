#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Imports from Vertex Shader
in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

// Textures
uniform sampler2D tex0;
uniform sampler2D tex1;

// Camera
uniform vec3 camPos;

// Material
struct MaterialData {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float shininess;
    bool useTexture;
};
uniform MaterialData material;

// Point Lights
#define MAX_POINT_LIGHTS 16
struct PointLight {
    vec3 position;
    vec4 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int pointLightCount;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform DirectionalLight sunLight;
uniform DirectionalLight moonLight;

vec3 GetBaseColor() {
    if (material.useTexture) {
        return vec3(texture(tex0, texCoord)) * material.albedo;
    }
    return material.albedo;
}

float GetSpecularStrength() {
    if (material.useTexture) {
        return texture(tex1, texCoord).r * (1.0 - material.roughness);
    }
    return (1.0 - material.roughness);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.direction);
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    vec3 baseColor = GetBaseColor();
    float specStrength = GetSpecularStrength();
    
    vec3 diffuse = light.color * diff * baseColor;
    vec3 specular = light.color * spec * specStrength * mix(vec3(1.0), baseColor, material.metallic);
    
    return (diffuse + specular) * light.intensity;
}

void main()
{
    // Normal and View direction
    vec3 normal = normalize(Normal);
    vec3 viewDirection = normalize(camPos - crntPos);

    vec3 baseColor = GetBaseColor();

    // Accumulate Lighting
    vec3 totalLighting = vec3(0.0);
    
    // 1. Ambient (affected by AO)
    float ambientStrength = 0.10;
    totalLighting += ambientStrength * baseColor * material.ao;

    // 2. Directional Lights (Sun & Moon)
    if (sunLight.intensity > 0.0)
        totalLighting += CalcDirLight(sunLight, normal, viewDirection);
    if (moonLight.intensity > 0.0)
        totalLighting += CalcDirLight(moonLight, normal, viewDirection);

    // 3. Point Lights
    int count = pointLightCount;
    if (count > MAX_POINT_LIGHTS) count = MAX_POINT_LIGHTS;
    
    for(int i = 0; i < count; ++i) {
        PointLight light = pointLights[i];
        
        vec3 lightVec = light.position - crntPos;
        float dist = length(lightVec);
        float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
        
        vec3 lightDirection = normalize(lightVec);
        
        // Diffuse
        float diffuse = max(dot(normal, lightDirection), 0.0);
        
        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDirection + viewDirection);
        float specAmount = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        float specStrength = GetSpecularStrength();
        
        // Combine with material
        vec3 diffuseColor = baseColor * diffuse * vec3(light.color);
        vec3 specularColor = specStrength * specAmount * vec3(light.color) * mix(vec3(1.0), baseColor, material.metallic);
        
        totalLighting += (diffuseColor + specularColor) * light.intensity * attenuation;
    }

    FragColor = vec4(totalLighting, 1.0);
}