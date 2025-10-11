#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float waveSpeed;
uniform float waveStrength;
uniform int waveSystem;

out vec3 FragPos;
out vec3 WorldPos;
out float WaveHeight;
out vec2 TexCoord;
out vec3 TangentLightDir;

// Blinn-Wyvill approximation for smooth wave profiles
float blinnWyvill(float x) {
    float x2 = x * x;
    return (1.0 - x2) * (1.0 - x2) * (1.0 - x2);
}

// Gerstner wave function for more realistic ocean waves
vec3 gerstnerWave(vec2 pos, vec2 direction, float amplitude, float wavelength, float speed, float time) {
    float k = 2.0 * 3.14159 / wavelength;
    float c = speed;
    vec2 d = normalize(direction);
    float f = k * (dot(d, pos) - c * time);
    float steepness = 0.6;
    
    return vec3(
        d.x * amplitude * cos(f) * steepness,
        amplitude * sin(f),
        d.y * amplitude * cos(f) * steepness
    );
}

// Combined wave system using multiple Gerstner waves
vec3 calculateWaves(vec2 pos, float time) {
    vec3 wave = vec3(0.0);
    
    // Large ocean swells
    wave += gerstnerWave(pos, vec2(1.0, 0.3), 0.15, 8.0, 2.0, time);
    wave += gerstnerWave(pos, vec2(-0.7, 0.9), 0.12, 6.5, 1.8, time);
    
    // Medium waves
    wave += gerstnerWave(pos, vec2(0.5, -1.0), 0.08, 4.0, 2.5, time);
    wave += gerstnerWave(pos, vec2(-0.9, -0.4), 0.06, 3.5, 2.2, time);
    
    // Small detail waves
    wave += gerstnerWave(pos, vec2(0.8, 0.6), 0.04, 2.0, 3.0, time);
    wave += gerstnerWave(pos, vec2(-0.3, -0.8), 0.03, 1.5, 3.5, time);
    
    return wave;
}

// Blinn-Wyvill based wave system (alternative, smoother style)
float getWaveHeight(vec2 pos, float time) {
    vec2 w1 = vec2(pos.x + pos.y * 2.5, pos.y - pos.x * 1.8) * 0.6;
    vec2 w2 = vec2(pos.x * 1.3 - pos.y * 3.2, pos.y * 1.5 + pos.x * 2.8) * 0.4;
    vec2 w3 = vec2(pos.x * 2.2 + pos.y * 1.7, pos.y * 1.9 - pos.x * 2.3) * 0.5;
    vec2 w4 = vec2(pos.x * 3.5 - pos.y * 0.9, pos.y * 2.7 + pos.x * 1.2) * 0.7;
    vec2 w5 = vec2(pos.x * 4.1 + pos.y * 0.5, pos.y * 3.3 - pos.x * 1.6) * 0.8;
    
    float t1 = mod(w1.x + time * 0.8, 2.0) - 1.0;
    float t2 = mod(w2.y - time * 1.2, 2.0) - 1.0;
    float t3 = mod(w3.x + time * 0.6, 2.0) - 1.0;
    float t4 = mod(w4.y - time * 1.5, 2.0) - 1.0;
    float t5 = mod(w5.x + time * 0.9, 2.0) - 1.0;
    
    float wave1 = blinnWyvill(t1) * 0.22;
    float wave2 = blinnWyvill(t2) * 0.16;
    float wave3 = blinnWyvill(t3) * 0.11;
    float wave4 = blinnWyvill(t4) * 0.07;
    float wave5 = blinnWyvill(t5) * 0.05;
    
    return wave1 + wave2 + wave3 + wave4 + wave5;
}

void main()
{
    vec3 pos = aPos;
    
    // Choose wave system (uncomment one):
    // Option 1: Gerstner waves (more realistic ocean)
    float waveHeight = 0.0;
    if (waveSystem == 0) {
        // Blinn-Wyvill waves
        waveHeight = getWaveHeight(pos.xz, time * waveSpeed) * waveStrength;
        pos.y += waveHeight;
    } else {
        // Gerstner waves
        vec3 waveDisplacement = calculateWaves(pos.xz, time * waveSpeed) * waveStrength;
        pos += waveDisplacement;
        waveHeight = waveDisplacement.y;
    }
    
    // Transform to world space
    WorldPos = vec3(model * vec4(pos, 1.0));
    FragPos = pos;
    WaveHeight = waveHeight;
    TexCoord = aTexCoord;
    
    // Calculate tangent space for better normal mapping (optional)
    vec3 tangent = normalize(vec3(1.0, 0.0, 0.0));
    vec3 normal = normalize(vec3(0.0, 1.0, 0.0));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    TangentLightDir = TBN * normalize(vec3(0.5, 1.0, 0.3));
    
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}