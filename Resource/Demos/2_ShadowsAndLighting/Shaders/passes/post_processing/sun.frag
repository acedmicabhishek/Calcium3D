#version 330 core
out vec4 FragColor;

in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;

uniform vec4 sunColor;
uniform float sunIntensity;
uniform vec3 camPos;

void main()
{
    // Need a rework
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(camPos - crntPos);
    
    // Fresnel effect for edge glow
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);
    
    // Core brightness
    float coreBrightness = 1.0;
    
    // Combine core and edge glow
    float brightness = coreBrightness + fresnel * 0.5;
    
    // Apply sun color and intensity
    vec3 finalColor = sunColor.rgb * brightness * sunIntensity;
    
    // Add some atmospheric scattering effect
    float distance = length(camPos - crntPos);
    float atmospheric = 1.0 / (1.0 + distance * 0.01);
    
    FragColor = vec4(finalColor * atmospheric, 1.0);
}

