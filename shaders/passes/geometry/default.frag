#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Imports from Vertex Shader
in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;
in vec4 FragPosLightSpace;

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
    int shadowIndex;
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

// Shadows
uniform int enableShadows;
uniform int enablePointShadows;
uniform float shadowBias;
uniform float pointShadowFarPlane;
uniform sampler2D dirShadowMap;
uniform samplerCube pointShadowMap0;
uniform samplerCube pointShadowMap1;
uniform samplerCube pointShadowMap2;
uniform samplerCube pointShadowMap3;

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

float DirShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    if (enableShadows == 0) return 0.0;
    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    // Avoid shadowing outside frustum
    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), shadowBias * 0.1);  
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(dirShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(dirShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}

float PointShadowCalculation(vec3 fragPos, vec3 lightPos, int index)
{
    if (enablePointShadows == 0) return 0.0;
    if (index < 0 || index >= 4) return 0.0;

    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    float shadow = 0.0;
    float bias = shadowBias * 15.0; // Needs more bias
    int samples = 20;
    float viewDistance = length(camPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / pointShadowFarPlane)) / 25.0;
    
    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );
    
    for(int i = 0; i < samples; ++i)
    {
        vec3 sampleDir = fragToLight + sampleOffsetDirections[i] * diskRadius;
        float closestDepth = 1.0;
        if (index == 0) closestDepth = texture(pointShadowMap0, sampleDir).r;
        else if (index == 1) closestDepth = texture(pointShadowMap1, sampleDir).r;
        else if (index == 2) closestDepth = texture(pointShadowMap2, sampleDir).r;
        else if (index == 3) closestDepth = texture(pointShadowMap3, sampleDir).r;
        
        closestDepth *= pointShadowFarPlane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    return shadow;
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
    if (sunLight.intensity > 0.0) {
        vec3 sunBase = CalcDirLight(sunLight, normal, viewDirection);
        float shadow = DirShadowCalculation(FragPosLightSpace, normal, normalize(sunLight.direction));
        totalLighting += sunBase * (1.0 - shadow);
    }
    if (moonLight.intensity > 0.0) {
        vec3 moonBase = CalcDirLight(moonLight, normal, viewDirection);
        totalLighting += moonBase;
    }

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
        
        float shadow = 0.0;
        if (light.shadowIndex >= 0) {
            shadow = PointShadowCalculation(crntPos, light.position, light.shadowIndex);
        }
        
        totalLighting += (diffuseColor + specularColor) * light.intensity * attenuation * (1.0 - shadow);
    }

    FragColor = vec4(totalLighting, 1.0);
}