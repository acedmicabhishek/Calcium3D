#version 330 core
out vec4 FragColor;

in vec3 nearPoint;
in vec3 farPoint;

uniform vec3 cameraPosition;
uniform float time;
uniform float cloudHeight;
uniform float farPlane;
uniform float density;
uniform float stepSize;
uniform float cloudCover;
uniform float speed;
uniform int quality; // 0 for Performance, 1 for Quality
uniform float detail;

// --- Optimized 3D Perlin Noise ---
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float pnoise(vec3 P) {
    vec3 Pi0 = floor(P);
    vec3 Pf0 = fract(P);
    vec3 Pi1 = Pi0 + vec3(1.0);
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    vec3 Pf1 = Pf0 - vec3(1.0);
    
    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    vec4 iy = vec4(Pi0.yy, Pi1.yy);
    vec4 iz0 = Pi0.zzzz;
    vec4 iz1 = Pi1.zzzz;

    vec4 ixy = permute(permute(ix) + iy);
    vec4 ixy0 = permute(ixy + iz0);
    vec4 ixy1 = permute(ixy + iz1);

    vec4 gx0 = ixy0 * (1.0 / 7.0);
    vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
    gx0 = fract(gx0);
    vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
    vec4 sz0 = step(gz0, vec4(0.0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    vec4 gx1 = ixy1 * (1.0 / 7.0);
    vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
    gx1 = fract(gx1);
    vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
    vec4 sz1 = step(gz1, vec4(0.0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

    vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
    vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
    vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
    vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
    vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
    vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
    vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
    vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

    vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);

    vec3 fade = Pf0 * Pf0 * Pf0 * (Pf0 * (Pf0 * 6.0 - 15.0) + 10.0);
    vec4 n_x = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade.z);
    vec2 n_xy = mix(n_x.xy, n_x.zw, fade.y);
    float n_xyz = mix(n_xy.x, n_xy.y, fade.x);
    return 2.2 * n_xyz;
}

// Optimized FBM with early exit
float fbm(vec3 p, int octaves) {
    float f = 0.0;
    float amp = 0.5;
    float totalAmp = 0.0;
    
    for (int i = 0; i < octaves; i++) {
        f += amp * pnoise(p);
        totalAmp += amp;
        p *= 2.0;
        amp *= 0.5;
    }
    return f / totalAmp; // Normalize
}

// Fast cloud density calculation with LOD
float getCloudDensity(vec3 pos, float distanceFromCamera) {
    vec3 movingPos = pos + vec3(time * speed, 0.0, time * speed * 0.5);
    
    // LOD based on distance - fewer octaves for distant clouds
    int octaves = (distanceFromCamera > farPlane * 0.5) ? max(1, int(detail) - 2) : int(detail);
    
    float noise = fbm(movingPos * 0.08, octaves);
    
    // Sharper cloud edges with adjusted coverage
    float coverage = cloudCover * 0.5 + 0.3;
    noise = smoothstep(coverage - 0.1, coverage + 0.1, noise);

    // Improved vertical shaping with softer falloff
    float heightDist = abs(pos.y - cloudHeight);
    float height_factor = exp(-heightDist * heightDist * 0.02);

    return noise * height_factor * density;
}

// Ray-sphere intersection for cloud layer bounds
bool intersectCloudLayer(vec3 ro, vec3 rd, out float tMin, out float tMax) {
    float cloudThickness = 15.0;
    float bottomY = cloudHeight - cloudThickness * 0.5;
    float topY = cloudHeight + cloudThickness * 0.5;
    
    float t1 = (bottomY - ro.y) / rd.y;
    float t2 = (topY - ro.y) / rd.y;
    
    tMin = min(t1, t2);
    tMax = max(t1, t2);
    
    tMin = max(tMin, 0.0);
    tMax = min(tMax, farPlane);
    
    return tMax > tMin;
}

void main()
{
    vec3 rayDir = normalize(farPoint - nearPoint);
    vec3 rayOrigin = cameraPosition;

    // Early exit if not looking at cloud layer
    float tMin, tMax;
    if (!intersectCloudLayer(rayOrigin, rayDir, tMin, tMax)) {
        FragColor = vec4(0.0);
        return;
    }

    // Adaptive step count
    int numSteps = (quality == 0) ? 32 : 64;
    float rayLength = tMax - tMin;
    float actualStepSize = rayLength / float(numSteps);
    
    float totalDensity = 0.0;
    float transmittance = 1.0;
    vec3 accumulatedColor = vec3(0.0);
    
    // Sun direction for lighting
    vec3 sunDir = normalize(vec3(0.3, 0.7, 0.5));
    vec3 sunColor = vec3(1.0, 0.95, 0.8);
    vec3 skyColor = vec3(0.6, 0.75, 0.95);

    for (int i = 0; i < numSteps; i++) {
        if (transmittance < 0.01) break; // Early exit when opaque
        
        float t = tMin + float(i) * actualStepSize;
        vec3 samplePos = rayOrigin + rayDir * t;
        
        float distFromCamera = length(samplePos - rayOrigin);
        float d = getCloudDensity(samplePos, distFromCamera);
        
        if (d > 0.01) {
            // Simple lighting - blend between sun and ambient
            float lightDir = dot(sunDir, -rayDir);
            float powderEffect = 1.0 - exp(-d * 2.0);
            float scattering = mix(0.5, 1.0, smoothstep(-0.5, 0.5, lightDir)) * powderEffect;
            
            vec3 cloudColor = mix(skyColor * 0.6, sunColor, scattering);
            
            float densityStep = d * actualStepSize;
            float sampleTransmittance = exp(-densityStep * 3.0);
            
            accumulatedColor += cloudColor * transmittance * (1.0 - sampleTransmittance);
            transmittance *= sampleTransmittance;
        }
    }
    
    totalDensity = 1.0 - transmittance;

    FragColor = vec4(accumulatedColor, totalDensity);
}