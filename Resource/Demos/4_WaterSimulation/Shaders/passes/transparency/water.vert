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
uniform float tiling;
uniform float surfaceHeight;

out vec3 FragPos;
out vec3 WorldPos;
out vec3 OriginalWorldPos;
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
    wave += gerstnerWave(pos, vec2(1.0, 0.3), 1.5, 80.0, 4.0, time);
    wave += gerstnerWave(pos, vec2(-0.7, 0.9), 1.2, 65.0, 3.6, time);
    
    // Medium waves
    wave += gerstnerWave(pos, vec2(0.5, -1.0), 0.8, 40.0, 5.0, time);
    wave += gerstnerWave(pos, vec2(-0.9, -0.4), 0.6, 35.0, 4.4, time);
    
    // Small detail waves (kept minimal for vertex pass)
    wave += gerstnerWave(pos, vec2(0.8, 0.6), 0.4, 20.0, 6.0, time);
    wave += gerstnerWave(pos, vec2(-0.3, -0.8), 0.3, 15.0, 7.0, time);
    
    return wave;
}

// Blinn-Wyvill based wave system (smoother style)
float getWaveHeight(vec2 pos, float time) {
    vec2 p = pos * 0.05; // Make the waves MUCH wider so they don't alias on the grid
    vec2 w1 = vec2(p.x + p.y * 2.5, p.y - p.x * 1.8) * 0.6;
    vec2 w2 = vec2(p.x * 1.3 - p.y * 3.2, p.y * 1.5 + p.x * 2.8) * 0.4;
    vec2 w3 = vec2(p.x * 2.2 + p.y * 1.7, p.y * 1.9 - p.x * 2.3) * 0.5;
    vec2 w4 = vec2(p.x * 3.5 - p.y * 0.9, p.y * 2.7 + p.x * 1.2) * 0.7;
    vec2 w5 = vec2(p.x * 4.1 + p.y * 0.5, p.y * 3.3 - p.x * 1.6) * 0.8;
    
    float t1 = mod(w1.x + time * 0.8, 2.0) - 1.0;
    float t2 = mod(w2.y - time * 1.2, 2.0) - 1.0;
    float t3 = mod(w3.x + time * 0.6, 2.0) - 1.0;
    float t4 = mod(w4.y - time * 1.5, 2.0) - 1.0;
    float t5 = mod(w5.x + time * 0.9, 2.0) - 1.0;
    
    // Taller amplitudes to match the wider spread
    float wave1 = blinnWyvill(t1) * 2.2;
    float wave2 = blinnWyvill(t2) * 1.6;
    float wave3 = blinnWyvill(t3) * 1.1;
    float wave4 = blinnWyvill(t4) * 0.7;
    float wave5 = blinnWyvill(t5) * 0.5;
    
    return wave1 + wave2 + wave3 + wave4 + wave5;
}

void main()
{
    // Transform to world space first to calculate waves independent of model scale
    WorldPos = vec3(model * vec4(aPos, 1.0));
    OriginalWorldPos = WorldPos;
    
    // Choose wave system (uncomment one):
    float waveHeight = 0.0;
    vec2 tiledPos = WorldPos.xz * tiling;
    
    if (waveSystem == 0) {
        // Ocean Swell (Blinn-Wyvill waves)
        waveHeight = getWaveHeight(tiledPos, time * waveSpeed) * waveStrength;
        WorldPos.y += waveHeight + surfaceHeight;
    } else if (waveSystem == 1) {
        // Ocean Swell (Gerstner waves)
        vec3 waveDisplacement = calculateWaves(tiledPos, time * waveSpeed) * waveStrength;
        WorldPos += waveDisplacement;
        WorldPos.y += surfaceHeight;
        waveHeight = waveDisplacement.y;
    } else if (waveSystem == 2) {
        // Micro-Ripples (Blinn-Wyvill, bypasses 0.05 size modifier inside getWaveHeight by prescaling)
        waveHeight = getWaveHeight(tiledPos * 20.0, time * waveSpeed) * waveStrength * 0.1;
        WorldPos.y += waveHeight + surfaceHeight;
    }
    
    FragPos = WorldPos; // Let FragPos and WorldPos be the same for simplicity
    OriginalWorldPos.y += surfaceHeight; // Keep OriginalWorldPos in sync for normal sampling
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
