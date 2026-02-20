#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ViewPos;

vec3 permute(vec3 x) { 
    return mod(((x * 34.0) + 1.0) * x, 289.0); 
}

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, 
                        -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + 
                     i.x + vec3(0.0, i1.x, 1.0));
    
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), 
                 dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    
    vec3 g;
    g.x  = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    
    return 130.0 * dot(m, g);
}

// Improved FBM with configurable octaves
float fbm(vec2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    float maxValue = 0.0;
    
    for(int i = 0; i < octaves; i++) {
        value += amplitude * snoise(p * frequency);
        maxValue += amplitude;
        amplitude *= 0.5;
        frequency *= 2.02 + float(i) * 0.01;
    }
    
    return value / maxValue;
}

// Fast FBM for detail
float fbm4(vec2 p) {
    float f = 0.0;
    f += 0.5000 * snoise(p); p *= 2.02;
    f += 0.2500 * snoise(p); p *= 2.03;
    f += 0.1250 * snoise(p); p *= 2.01;
    f += 0.0625 * snoise(p);
    return f / 0.9375;
}
uniform float u_time;
uniform vec2 u_resolution;
uniform vec3 u_cloudColor;
uniform float u_cloudCover;      // 0.0 - 1.0
uniform float u_cloudSpeed;      // Recommend: 0.5 - 2.0
uniform float u_tiling;          // Recommend: 1.0 - 5.0
uniform float u_density;         // 0.0 - 1.0
uniform float u_cloudSize;       // Recommend: 0.5 - 3.0
uniform float u_randomness;      // 0.0 - 1.0


uniform vec3 u_lightDir = vec3(0.5, 1.0, 0.3);
uniform float u_lightIntensity = 0.3;
uniform vec3 u_ambientColor = vec3(0.9, 0.95, 1.0);

void main()
{
    // Use world position for stable, tiling clouds
    vec2 uv = WorldPos.xz * 0.01 * u_tiling;
    vec2 motion = vec2(u_time * u_cloudSpeed * 0.1, 0.0);
    
    // Controls overall cloud distribution
    float weatherNoise = fbm4(uv * 0.5 + motion * 0.5);
    float weatherMap = weatherNoise * 0.5 + 0.5; // Remap to 0-1
    
    // Dynamic coverage based on weather map
    float coverage = mix(u_cloudCover - 0.1, u_cloudCover + 0.1, 
                        weatherMap * u_randomness);
    coverage = clamp(coverage, 0.0, 1.0);
    
    float baseNoise = fbm4(uv * u_cloudSize + motion);
    baseNoise = baseNoise * 0.5 + 0.5; // Remap to 0-1
    
    // Apply coverage threshold
    float cloudShape = smoothstep(1.0 - coverage, 1.0, baseNoise);
    
    // Early discard for performance
    if (cloudShape < 0.01) discard;
    
    // Add fine detail to cloud edges
    float detailNoise = fbm4(uv * u_cloudSize * 3.5 + motion * 1.8);
    detailNoise = detailNoise * 0.5 + 0.5;
    

    float detailMask = smoothstep(0.3, 0.7, detailNoise);
    cloudShape *= mix(1.0, detailMask, 0.6);

    // Simulate volumetric lighting effect
    vec3 lightDir = normalize(u_lightDir);
    
    // Thickness-based lighting (thicker clouds are darker underneath)
    float thickness = cloudShape * baseNoise;
    float lightPenetration = exp(-thickness * 2.0);
    
    // Soft lighting gradient
    float lightAmount = mix(0.6, 1.0, lightPenetration) * (1.0 + u_lightIntensity);
    
    // Add rim lighting effect on cloud edges
    float edgeMask = 1.0 - smoothstep(0.0, 0.3, cloudShape);
    float rimLight = edgeMask * 0.4;

    vec3 cloudLit = u_cloudColor * u_ambientColor * lightAmount;
    cloudLit += rimLight * vec3(1.0, 1.0, 1.0);
    
    // Add subtle color variation based on noise
    float colorVariation = fbm4(uv * u_cloudSize * 0.3 + motion * 0.3);
    cloudLit *= 1.0 + colorVariation * 0.1;

    float alpha = cloudShape * u_density;
    alpha = smoothstep(0.0, 0.1, alpha);
    
    if (alpha < 0.01) discard;
    
    FragColor = vec4(cloudLit, alpha);
}