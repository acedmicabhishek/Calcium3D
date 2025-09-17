#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 topColor;
uniform vec3 bottomColor;
uniform vec3 DynamicSunPos;
uniform vec3 DynamicMoonPos;
uniform vec3 sunColor;
uniform vec3 moonColor;

void main()
{
    float t = normalize(FragPos).y * 0.5 + 0.5;
    vec3 finalColor = mix(bottomColor, topColor, t);

    // Sun
    float sunDiskSize = 0.02;
    float sunDistance = distance(normalize(FragPos), normalize(DynamicSunPos));
    if (sunDistance < sunDiskSize)
    {
        finalColor = sunColor;
    }

    // Moon
    float moonDiskSize = 0.015;
    float moonDistance = distance(normalize(FragPos), normalize(DynamicMoonPos));
    if (moonDistance < moonDiskSize)
    {
        finalColor = moonColor;
    }

    FragColor = vec4(finalColor, 1.0);
}
