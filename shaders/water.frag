#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 WorldPos;
in float WaveHeight;

uniform float time;
uniform vec3 viewPos;
uniform float waveSpeed;
uniform float waveStrength;
uniform float shininess;
uniform vec3 waterColor;

// Blinn-Wyvill approximation
float blinnWyvill(float x) {
    float x2 = x * x;
    return (1.0 - x2) * (1.0 - x2) * (1.0 - x2);
}

// Enhanced wave height function (matches vertex shader)
float getWaveHeight(vec2 pos, float time_param) {
    vec2 w1 = vec2(pos.x + pos.y * 2.5, pos.y - pos.x * 1.8) * 0.6;
    vec2 w2 = vec2(pos.x * 1.3 - pos.y * 3.2, pos.y * 1.5 + pos.x * 2.8) * 0.4;
    vec2 w3 = vec2(pos.x * 2.2 + pos.y * 1.7, pos.y * 1.9 - pos.x * 2.3) * 0.5;
    vec2 w4 = vec2(pos.x * 3.5 - pos.y * 0.9, pos.y * 2.7 + pos.x * 1.2) * 0.7;
    
    float t1 = mod(w1.x + time_param * 0.8, 2.0) - 1.0;
    float t2 = mod(w2.y - time_param * 1.2, 2.0) - 1.0;
    float t3 = mod(w3.x + time_param * 0.6, 2.0) - 1.0;
    float t4 = mod(w4.y - time_param * 1.5, 2.0) - 1.0;
    
    float wave1 = blinnWyvill(t1) * 0.25;
    float wave2 = blinnWyvill(t2) * 0.18;
    float wave3 = blinnWyvill(t3) * 0.12;
    float wave4 = blinnWyvill(t4) * 0.08;
    
    return wave1 + wave2 + wave3 + wave4;
}

// Improved normal calculation with better sampling
vec3 calculateNormal(vec3 pos, float delta, float time) {
    vec2 p_xz = pos.xz;
    
    float h0 = getWaveHeight(p_xz, time);
    float hx = getWaveHeight(p_xz + vec2(delta, 0.0), time);
    float hz = getWaveHeight(p_xz + vec2(0.0, delta), time);
    
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
    vec3 normal = calculateNormal(FragPos, 0.015, time * waveSpeed);
    
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
    
    // Water colors (deep to shallow)
    vec3 deepWater = waterColor * 0.5;
    vec3 shallowWater = waterColor;
    vec3 finalWaterColor = mix(deepWater, shallowWater, fresnelTerm * 0.5 + 0.5);
    
    // Combine lighting components
    vec3 ambient = finalWaterColor * 0.3;
    vec3 diffuseColor = finalWaterColor * diffuse * 0.6;
    vec3 specularColor = vec3(1.0, 1.0, 0.95) * spec * 0.8;
    vec3 fresnelColor = vec3(0.7, 0.85, 1.0) * fresnelTerm * 0.3;
    
    // Add foam highlights on wave peaks
    float foamIntensity = smoothstep(0.1, 0.3, abs(WaveHeight));
    vec3 foam = vec3(0.9, 0.95, 1.0) * foamIntensity * 0.2;
    
    // Final color composition
    vec3 finalColor = ambient + diffuseColor + specularColor + fresnelColor + foam;
    
    // Add subtle underwater caustics effect
    
    // Adjust transparency based on viewing angle
    float alpha = mix(0.7, 0.95, fresnelTerm);
    
    FragColor = vec4(finalColor, alpha);
}