#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 topColor;
uniform vec3 bottomColor;
uniform vec3 DynamicSunPos;
uniform vec3 DynamicMoonPos;
uniform vec3 sunColor;
uniform vec3 moonColor;
uniform float sunBloom;
uniform float moonBloom;

void main()
{
    float t = normalize(FragPos).y * 0.5 + 0.5;
    vec3 finalColor = mix(bottomColor, topColor, t);

    // Sun
    float sunDiskSize = 0.02;
    float sunDistance = distance(normalize(FragPos), normalize(DynamicSunPos));
    float sunGlow = smoothstep(sunDiskSize + sunBloom, sunDiskSize, sunDistance);
    finalColor = mix(finalColor, sunColor, sunGlow);

    // Moon
    float moonDiskSize = 0.015;
    float moonDistance = distance(normalize(FragPos), normalize(DynamicMoonPos));
    float moonGlow = smoothstep(moonDiskSize + moonBloom, moonDiskSize, moonDistance);
    finalColor = mix(finalColor, moonColor, moonGlow);

    FragColor = vec4(finalColor, 1.0);
}
