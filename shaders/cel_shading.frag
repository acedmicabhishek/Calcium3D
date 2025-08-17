#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec4 lightColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    float intensity;
    if (diff > 0.95)
        intensity = 1.0;
    else if (diff > 0.5)
        intensity = 0.6;
    else if (diff > 0.25)
        intensity = 0.4;
    else
        intensity = 0.2;
        
    vec3 result = intensity * lightColor.rgb;
    FragColor = vec4(result, 1.0);
}