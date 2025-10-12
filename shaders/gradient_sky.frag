#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 ViewDir;

uniform vec3 topColor;
uniform vec3 bottomColor;
uniform vec3 DynamicSunPos;
uniform vec3 DynamicMoonPos;
uniform vec3 sunColor;
uniform vec3 moonColor;
uniform float sunBloom;
uniform float moonBloom;
uniform float u_time;

// Improved hash function
float hash(vec2 p) {
    p = fract(p * vec2(5.3987, 5.4421));
    p += dot(p.yx, p.xy + vec2(21.5351, 14.3137));
    return fract(p.x * p.y * 95.4307);
}

// Better noise function
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Atmospheric gradient with horizon glow
vec3 atmosphericGradient(float h, vec3 top, vec3 bottom) {
    // Enhanced gradient with horizon emphasis
    float t = pow(h * 0.5 + 0.5, 0.8);
    vec3 skyColor = mix(bottom, top, t);
    
    // Add subtle horizon glow
    float horizonGlow = exp(-abs(h) * 5.0) * 0.3;
    skyColor += vec3(horizonGlow * 0.5, horizonGlow * 0.3, horizonGlow * 0.1);
    
    return skyColor;
}

// Improved celestial body rendering
float celestialDisk(vec3 dir, vec3 bodyPos, float diskSize, float bloomSize) {
    float dist = distance(dir, normalize(bodyPos));
    
    // Sharp disk
    float disk = 1.0 - smoothstep(diskSize * 0.95, diskSize, dist);
    
    // Smooth bloom
    float bloom = exp(-dist * dist / (bloomSize * bloomSize)) * 0.5;
    
    return disk + bloom;
}

// Enhanced star field
float starField(vec2 uv, float time) {
    // Multiple layers of stars with different densities
    float stars = 0.0;
    
    // Large stars
    vec2 uv1 = uv * 500.0;
    float n1 = hash(uv1);
    stars += smoothstep(0.9985, 1.0, n1) * (noise(uv1 * 0.1 + time * 0.1) * 0.5 + 0.75);
    
    // Medium stars
    vec2 uv2 = uv * 1200.0;
    float n2 = hash(uv2);
    stars += smoothstep(0.9992, 1.0, n2) * (noise(uv2 * 0.1 + time * 0.15) * 0.4 + 0.7) * 0.7;
    
    // Small stars
    vec2 uv3 = uv * 2000.0;
    float n3 = hash(uv3);
    stars += smoothstep(0.9996, 1.0, n3) * (noise(uv3 * 0.1 + time * 0.2) * 0.3 + 0.8) * 0.5;
    
    return stars;
}

void main() {
    vec3 dir = normalize(ViewDir);
    float height = dir.y;
    
    // Base atmospheric color
    vec3 finalColor = atmosphericGradient(height, topColor, bottomColor);
    
    // Sun rendering
    float sunDiskSize = 0.02;
    float sunIntensity = celestialDisk(dir, DynamicSunPos, sunDiskSize, sunBloom);
    vec3 sunContribution = sunColor * sunIntensity;
    
    // Add sun halo
    float sunDist = distance(dir, normalize(DynamicSunPos));
    float sunHalo = exp(-sunDist * 3.0) * 0.15;
    sunContribution += sunColor * sunHalo;
    
    finalColor += sunContribution;
    
    // Moon rendering with crater detail
    float moonDiskSize = 0.015;
    float moonIntensity = celestialDisk(dir, DynamicMoonPos, moonDiskSize, moonBloom);
    
    // Add subtle crater texture
    vec2 moonUV = (dir.xy - normalize(DynamicMoonPos).xy) * 100.0;
    float craters = noise(moonUV * 5.0) * 0.3;
    vec3 moonSurface = moonColor * (0.8 + craters * 0.2);
    
    finalColor += moonSurface * moonIntensity;
    
    // Enhanced star rendering
    // Only visible above horizon and at night
    if (height > -0.1) {
        float nightFactor = 1.0 - smoothstep(0.0, 0.4, (topColor.r + topColor.g + topColor.b) / 3.0);
        nightFactor *= smoothstep(-0.1, 0.05, height);
        
        vec2 starUV = FragPos.xy;
        float stars = starField(starUV, u_time);
        
        // Fade stars near sun and moon
        float sunFade = 1.0 - smoothstep(0.1, 0.3, sunDist);
        float moonDist = distance(dir, normalize(DynamicMoonPos));
        float moonFade = 1.0 - smoothstep(0.05, 0.2, moonDist);
        
        stars *= (1.0 - sunFade * 0.8) * (1.0 - moonFade * 0.5);
        
        // Add star color variation
        vec3 starColor = vec3(1.0, 0.95, 0.9) + vec3(noise(starUV * 100.0)) * 0.1;
        finalColor += starColor * stars * nightFactor;
    }
    
    // Subtle color grading
    finalColor = pow(finalColor, vec3(1.05)); // Slight contrast boost
    
    FragColor = vec4(finalColor, 1.0);
}