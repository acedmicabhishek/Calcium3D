#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec4 lightColor;
// Sun (Global Light) uniforms
uniform vec4 sunColor;
uniform vec3 sunPos;
uniform float sunIntensity;

void main()
{
    vec3 norm = normalize(Normal);
    
    // Sun light calculation (primary light source)
    vec3 sunDir = normalize(sunPos - FragPos);
    float sunDiff = max(dot(norm, sunDir), 0.0) * sunIntensity;
    
    float sunIntensityValue;
    if (sunDiff > 0.95)
        sunIntensityValue = 1.0;
    else if (sunDiff > 0.5)
        sunIntensityValue = 0.6;
    else if (sunDiff > 0.25)
        sunIntensityValue = 0.4;
    else
        sunIntensityValue = 0.2;
        
    vec3 result = sunIntensityValue * sunColor.rgb;
    
    FragColor = vec4(result, 1.0);
}