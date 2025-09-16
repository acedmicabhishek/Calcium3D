#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 topColor;
uniform vec3 bottomColor;

void main()
{
    float t = normalize(FragPos).y * 0.5 + 0.5;
    vec3 finalColor = mix(bottomColor, topColor, t);
    FragColor = vec4(finalColor, 1.0);
}
