#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;


// Imports the color from the Vertex Shader
in vec3 color;
// Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;
// Imports the normal from the Vertex Shader
in vec3 Normal;
// Imports the current position from the Vertex Shader
in vec3 crntPos;

// Gets the Texture Unit from the main function
uniform sampler2D tex0;
uniform sampler2D tex1;
// Gets the color of the light from the main function
// uniform vec4 lightColor; // Unused in main()
// Gets the position of the light from the main function
// uniform vec3 lightPos; // Unused in main()
// Gets the position of the camera from the main function
uniform vec3 camPos;
// Sun/Environment uniforms (for ambient/sky)
// uniform vec4 sunColor; // Unused
// uniform bool sunEnabled; // Unused

// Point Lights
#define MAX_POINT_LIGHTS 16
struct PointLight {
    vec3 position;
    vec4 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    // bool enabled; // Implicitly if in the array/count, it's enabled. Or check intensity.
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

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.direction);
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Hardcoded shininess for now
    
    vec3 diffuse = light.color * diff * vec3(texture(tex0, texCoord));
    vec3 specular = light.color * spec * vec3(texture(tex1, texCoord).r);
    
    return (diffuse + specular) * light.intensity;
}

void main()
{
	// Ambient lighting (global)
	float ambientStrength = 0.10f;
    vec3 ambient = ambientStrength * vec3(1.0);

    // Normal and View direction
    vec3 normal = normalize(Normal);
    vec3 viewDirection = normalize(camPos - crntPos);

    // Accumulate Lighting
    vec3 totalLighting = vec3(0.0);
    
    // 1. Ambient
    totalLighting += ambient * vec3(texture(tex0, texCoord));

    // 2. Directional Lights (Sun & Moon)
    // Only calculate if intensity > 0
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
        float a = light.quadratic;
        float b = light.linear;
        float c = light.constant;
        float intensity = 1.0f / (a * dist * dist + b * dist + c);
        
        vec3 lightDirection = normalize(lightVec);
        
        // Diffuse
        float diffuse = max(dot(normal, lightDirection), 0.0f);
        
        // Specular
        float specularLight = 0.50f;
        vec3 reflectionDirection = reflect(-lightDirection, normal);
        float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
        float specular = specAmount * specularLight;
        
        // Combine
        vec3 diffuseColor = vec3(texture(tex0, texCoord)) * diffuse * vec3(light.color);
        vec3 specularColor = vec3(texture(tex1, texCoord).r) * specular * vec3(light.color);
        
        totalLighting += (diffuseColor + specularColor) * light.intensity * intensity;
    }

	FragColor = vec4(totalLighting, 1.0f);
}