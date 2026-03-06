#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalColor;

in vec3 FragPos;
in vec3 WorldPos;
in vec3 OriginalWorldPos;
in float WaveHeight;

uniform float time;
uniform vec3 viewPos;
uniform float waveSpeed;
uniform float waveStrength;
uniform float shininess;
uniform vec3 waterColor;
uniform int waveSystem;
uniform float tiling;

uniform sampler2D depthTexture;
uniform mat4 invView;
uniform mat4 invProjection;
uniform vec2 screenResolution;


vec3 worldPosFromDepth(float depth, vec2 uv) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = invView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

// Blinn-Wyvill approximation
float blinnWyvill(float x) {
    float x2 = x * x;
    return (1.0 - x2) * (1.0 - x2) * (1.0 - x2);
}

// Gerstner wave function
vec3 gerstnerWave(vec2 pos, vec2 direction, float amplitude, float wavelength, float speed, float time_param) {
    float k = 2.0 * 3.14159 / wavelength;
    float c = speed;
    vec2 d = normalize(direction);
    float f = k * (dot(d, pos) - c * time_param);
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
    // Small detail waves
    wave += gerstnerWave(pos, vec2(0.8, 0.6), 0.4, 20.0, 6.0, time);
    wave += gerstnerWave(pos, vec2(-0.3, -0.8), 0.3, 15.0, 7.0, time);
    
    return wave;
}

// Blinn-Wyvill based wave system (smoother style)
float getWaveHeight(vec2 pos, float time_param) {
    // Make the waves MUCH wider so they don't alias on the grid
    vec2 p = pos * 0.05;
    vec2 w1 = vec2(p.x + p.y * 2.5, p.y - p.x * 1.8) * 0.6;
    vec2 w2 = vec2(p.x * 1.3 - p.y * 3.2, p.y * 1.5 + p.x * 2.8) * 0.4;
    vec2 w3 = vec2(p.x * 2.2 + p.y * 1.7, p.y * 1.9 - p.x * 2.3) * 0.5;
    vec2 w4 = vec2(p.x * 3.5 - p.y * 0.9, p.y * 2.7 + p.x * 1.2) * 0.7;
    vec2 w5 = vec2(p.x * 4.1 + p.y * 0.5, p.y * 3.3 - p.x * 1.6) * 0.8;
    
    float t1 = mod(w1.x + time_param * 0.8, 2.0) - 1.0;
    float t2 = mod(w2.y - time_param * 1.2, 2.0) - 1.0;
    float t3 = mod(w3.x + time_param * 0.6, 2.0) - 1.0;
    float t4 = mod(w4.y - time_param * 1.5, 2.0) - 1.0;
    float t5 = mod(w5.x + time_param * 0.9, 2.0) - 1.0;
    
    // Taller amplitudes to match the wider spread
    float wave1 = blinnWyvill(t1) * 2.2;
    float wave2 = blinnWyvill(t2) * 1.6;
    float wave3 = blinnWyvill(t3) * 1.1;
    float wave4 = blinnWyvill(t4) * 0.7;
    float wave5 = blinnWyvill(t5) * 0.5;
    
    return wave1 + wave2 + wave3 + wave4 + wave5;
}

vec3 calculateNormal(vec3 pos, float delta, float time_param) {
    vec2 p_xz = pos.xz * tiling;
    
    float h0, hx, hz;
    if (waveSystem == 0) {
        h0 = getWaveHeight(p_xz, time_param) * waveStrength;
        hx = getWaveHeight(p_xz + vec2(delta, 0.0), time_param) * waveStrength;
        hz = getWaveHeight(p_xz + vec2(0.0, delta), time_param) * waveStrength;
    } else if (waveSystem == 1) {
        h0 = calculateWaves(p_xz, time_param).y * waveStrength;
        hx = calculateWaves(p_xz + vec2(delta, 0.0), time_param).y * waveStrength;
        hz = calculateWaves(p_xz + vec2(0.0, delta), time_param).y * waveStrength;
    } else if (waveSystem == 2) {
        h0 = getWaveHeight(p_xz * 20.0, time_param) * waveStrength * 0.1;
        hx = getWaveHeight((p_xz + vec2(delta, 0.0)) * 20.0, time_param) * waveStrength * 0.1;
        hz = getWaveHeight((p_xz + vec2(0.0, delta)) * 20.0, time_param) * waveStrength * 0.1;
    }
    
    // Use wider finite differences relative to wave sizing to avoid noise
    // But delta here provides the slope multiplier. 
    // Using a delta of 1.0 stabilizes normal direction since details are large.
    vec3 tangentX = normalize(vec3(delta, hx - h0, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, hz - h0, delta));
    
    return normalize(cross(tangentZ, tangentX));
}

// Fresnel effect (Schlick's approximation)
float fresnel(vec3 viewDir, vec3 normal, float power) {
    float cosTheta = max(dot(viewDir, normal), 0.0);
    return pow(1.0 - cosTheta, power);
}

void main()
{
    // Calculate normal with finer detail
    // We use a delta of 1.0 because our waves are large scale
    vec3 normal = calculateNormal(OriginalWorldPos, 1.0, time * waveSpeed);
    
    // Attenuate normals at a distance to prevent Moiré aliasing / zagged lines
    // Fade the normals much earlier: starting at 40m, complete at 200m
    float dist = length(viewPos - WorldPos);
    float normalAttenuation = clamp((dist - 40.0) / 160.0, 0.0, 1.0);
    normal = mix(normal, vec3(0.0, 1.0, 0.0), normalAttenuation);
    normal = normalize(normal);
    
    // Lighting setup
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Specular lighting (Blinn-Phong)
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Fresnel effect for edge highlights
    float fresnelTerm = fresnel(viewDir, normal, 3.0);
    
    // Calculate world position of the scene floor behind this pixel
    vec2 screenUV = gl_FragCoord.xy / screenResolution;
    float sceneDepth = texture(depthTexture, screenUV).r;
    vec3 floorPos = worldPosFromDepth(sceneDepth, screenUV);
    float thickness = length(floorPos - WorldPos);
    
    // Tint color based on thickness (Beer's Law approximation)
    float absorption = clamp(thickness * 0.15, 0.0, 1.0);
    vec3 thickWaterColor = mix(waterColor, vec3(0.0, 0.1, 0.2), absorption);
    
    // Water colors (deep to shallow)
    vec3 deepWater = thickWaterColor * 0.5;
    vec3 shallowWater = thickWaterColor;
    vec3 finalWaterColor = mix(deepWater, shallowWater, fresnelTerm * 0.5 + 0.5);
    
    // Combine lighting components
    vec3 ambient = finalWaterColor * 0.3;
    vec3 diffuseColor = finalWaterColor * diffuse * 0.6;
    vec3 specularColor = vec3(1.0, 1.0, 0.95) * spec * 0.8;
    vec3 fresnelColor = vec3(0.7, 0.85, 1.0) * fresnelTerm * 0.3;
    
    // Add foam highlights on wave peaks
    float foamIntensity = smoothstep(0.1, 0.3, abs(WaveHeight / 10.0));
    vec3 foam = vec3(0.9, 0.95, 1.0) * foamIntensity * 0.2;
    
    // Final color composition
    vec3 finalColor = ambient + diffuseColor + specularColor + fresnelColor + foam;
    
    // Adjust transparency based on viewing angle and thickness
    // Water becomes more opaque as it gets thicker
    float depthAlpha = clamp(thickness * 0.5, 0.0, 0.7);
    float alpha = mix(0.7 + depthAlpha, 0.95, fresnelTerm);
    
    FragColor = vec4(finalColor, alpha);
    
    // Output world-space surface normal to G-Buffer attachment 1 to permit SSR pass mapping
    NormalColor = vec4(normal, 0.85);
}