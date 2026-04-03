#version 430 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;


in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;
in vec4 FragPosLightSpace;

// Textures
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform bool debugZPrepass;
uniform bool debugVRS;
uniform int vrsMode;

// Camera
uniform vec3 camPos;
uniform float zNear;
uniform float zFar;
uniform vec2 screenSize;

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

// Clustered Lighting SSBOs
struct PointLightData {
    vec4 positionAndRadius;
    vec4 colorAndIntensity;
};

struct ClusterData {
    int count;
    int lightIndices[64];
};

layout(std140, binding = 0) buffer LightBuffer {
    PointLightData lights[];
};

layout(std140, binding = 1) buffer ClusterBuffer {
    ClusterData clusters[];
};

// Directional Light
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform DirectionalLight sunLight;
uniform DirectionalLight moonLight;

// Shadows
uniform int enableShadows;
uniform float shadowBias;
uniform sampler2D dirShadowMap;

const int CLUSTER_X = 16;
const int CLUSTER_Y = 9;
const int CLUSTER_Z = 24;

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
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    vec3 baseColor = GetBaseColor();
    float specStrength = GetSpecularStrength();
    
    vec3 diffuse = light.color * diff * baseColor;
    vec3 specular = light.color * spec * specStrength * mix(vec3(1.0), baseColor, material.metallic);
    
    return (diffuse + specular) * light.intensity;
}

float DirShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    
    float closestDepth = texture(dirShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(dirShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(dirShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

vec3 CalcPointLightStruct(PointLightData light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightPos = light.positionAndRadius.xyz;
    float radius = light.positionAndRadius.w;
    vec3 lightColor = light.colorAndIntensity.xyz;
    float intensity = light.colorAndIntensity.w;
    
    vec3 lightDir = normalize(lightPos - fragPos);
    float dist = length(lightPos - fragPos);
    
    if (dist > radius) return vec3(0.0);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    float attenuation = clamp(1.0 - (dist * dist) / (radius * radius), 0.0, 1.0);
    attenuation *= attenuation;
    
    vec3 baseColor = GetBaseColor();
    float specStrength = GetSpecularStrength();
    
    vec3 diffuse = lightColor * diff * baseColor;
    vec3 specular = lightColor * spec * specStrength * mix(vec3(1.0), baseColor, material.metallic);
    
    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);
    vec3 result = vec3(0.0);
    
    // Directional Lights
    vec3 sunResult = CalcDirLight(sunLight, normal, viewDir);
    vec3 moonResult = CalcDirLight(moonLight, normal, viewDir);
    
    float shadow = 0.0;
    if (enableShadows == 1) {
        shadow = DirShadowCalculation(FragPosLightSpace, normal, normalize(sunLight.direction));
    }
    
    result += sunResult * (1.0 - shadow) + moonResult;
    
    // Ambient
    result += GetBaseColor() * 0.1 * material.ao;
    
    // Clustered Point Lights Loop
    vec2 fragCoord = gl_FragCoord.xy;
    float zLinear = gl_FragCoord.z / gl_FragCoord.w;
    
    int clusterX = int(fragCoord.x / screenSize.x * CLUSTER_X);
    int clusterY = int(fragCoord.y / screenSize.y * CLUSTER_Y);
    
    float zScale = float(CLUSTER_Z) / log2(zFar / zNear);
    float zBias = -(float(CLUSTER_Z) * log2(zNear) / log2(zFar / zNear));
    int clusterZ = int(log2(zLinear) * zScale + zBias);
    
    clusterX = clamp(clusterX, 0, CLUSTER_X - 1);
    clusterY = clamp(clusterY, 0, CLUSTER_Y - 1);
    clusterZ = clamp(clusterZ, 0, CLUSTER_Z - 1);
    
    int clusterIndex = clusterX + (clusterY * CLUSTER_X) + (clusterZ * CLUSTER_X * CLUSTER_Y);
    
    ClusterData cluster = clusters[clusterIndex];
    
    bool skipExpensive = false;
    if (vrsMode == 1) {
        if (int(fragCoord.x) % 2 == 1) skipExpensive = true;
    } else if (vrsMode == 2) {
        if (int(fragCoord.y) % 2 == 1) skipExpensive = true;
    } else if (vrsMode == 3) {
        if (int(fragCoord.x) % 2 == 1 || int(fragCoord.y) % 2 == 1) skipExpensive = true;
    }

    if (!skipExpensive || debugVRS) {
        for (int i = 0; i < cluster.count; ++i) {
            int lightIdx = cluster.lightIndices[i];
            result += CalcPointLightStruct(lights[lightIdx], normal, crntPos, viewDir);
        }
    }
    
    if (debugZPrepass) {
        result = mix(result, vec3(0.0, 1.0, 0.0), 0.7);
    }
    if (debugVRS) {
        vec3 heatColor;
        if (vrsMode == 0) heatColor = vec3(1.0, 0.0, 0.0); 
        else if (vrsMode == 1 || vrsMode == 2) heatColor = vec3(1.0, 1.0, 0.0);
        else if (vrsMode == 3) heatColor = vec3(0.0, 1.0, 0.0);
        else heatColor = vec3(1.0, 0.0, 0.0);
        
        result = mix(result, heatColor, 0.7);
    }
    
    FragColor = vec4(result, 1.0);
    NormalColor = vec4(normal, material.metallic);
}
