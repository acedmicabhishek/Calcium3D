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
uniform float u_time;

// Hash function to generate pseudo-random numbers
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// Noise function for twinkling
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f*f*(3.0-2.0*f);
    float res = mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
                    mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
    return res;
}

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

    // Stars
    vec2 star_uv = FragPos.xy * 1000.0;
    float star_noise = hash(star_uv);
    float stars = smoothstep(0.998, 1.0, star_noise);

    // Twinkling
    stars *= (noise(star_uv + u_time * 0.1) * 0.5 + 0.5);

    // Only show stars at night (when topColor is dark)
    float night_factor = 1.0 - (topColor.r + topColor.g + topColor.b) / 3.0;
    stars *= smoothstep(0.5, 0.7, night_factor);

    finalColor += vec3(stars);

    FragColor = vec4(finalColor, 1.0);
}
