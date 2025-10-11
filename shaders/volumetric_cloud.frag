// this is a bad code btw

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

// --- 3D Classic Perlin Noise ---
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 permute(vec4 x) { return mod289(((x*34.0)+1.0)*x); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float pnoise(vec3 P) {
    vec3 Pi0 = floor(P); // Integer part for indexing
    vec3 Pf0 = fract(P); // Fractional part for interpolation
    vec3 Pi1 = Pi0 + vec3(1.0);
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
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

float fbm(vec3 p, int octaves) {
    float f = 0.0;
    float amp = 0.5;
    for (int i = 0; i < octaves; i++) {
        f += amp * pnoise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return f;
}

float getCloudDensity(vec3 pos) {
    vec3 movingPos = pos + vec3(time * speed, 0.0, time * speed * 0.5);
    
    float noise = fbm(movingPos * 0.1, int(detail));
    noise = smoothstep(1.0 - cloudCover, 1.0, noise);

    // Vertical shaping
    float height_factor = 1.0 - abs(pos.y - cloudHeight) / 10.0;
    height_factor = smoothstep(0.0, 1.0, height_factor);

    return noise * height_factor * density;
}

void main()
{
    vec3 rayDir = normalize(farPoint - nearPoint);
    vec3 rayOrigin = cameraPosition;

    float totalDensity = 0.0;
    
    int numSteps = (quality == 0) ? 64 : 128;

    for (int i = 0; i < numSteps; i++) {
        float t = float(i) * stepSize;
        if (t > farPlane) break;

        vec3 samplePos = rayOrigin + rayDir * t;
        
        float d = getCloudDensity(samplePos);
        if (d > 0.0) {
            totalDensity += d * stepSize * 0.1; // Adjusted density contribution
            if (totalDensity > 1.0) break;
        }
    }

    vec3 sunDir = normalize(vec3(0.3, 0.6, 0.5));
    float light = dot(sunDir, rayDir);
    light = smoothstep(0.0, 1.0, light);
    
    vec3 color = mix(vec3(0.7, 0.8, 0.9), vec3(1.0), light);

    FragColor = vec4(color, totalDensity);
}