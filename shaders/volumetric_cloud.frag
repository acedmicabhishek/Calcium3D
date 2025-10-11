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

// --- 3D Simplex Noise (GLSL) ---
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
float snoise(vec3 v){
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //  x0, x1, x2, x3
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;

    // Permutations
    i = mod(i, 289.0 );
    vec4 p = permute( permute( permute(
              i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
            + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
            + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients
    float n_ = 1.0/7.0;
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 g0 = vec3(a0.xy,h.x);
    vec3 g1 = vec3(a0.zw,h.y);
    vec3 g2 = vec3(a1.xy,h.z);
    vec3 g3 = vec3(a1.zw,h.w);

    vec4 norm = 1.79284291400159 - 0.85373472095314 *
        vec4(dot(g0,g0), dot(g1,g1), dot(g2,g2), dot(g3,g3));
    g0 *= norm.x;
    g1 *= norm.y;
    g2 *= norm.z;
    g3 *= norm.w;

    vec4 m = max(0.6 - vec4(
        dot(x0,x0),
        dot(x1,x1),
        dot(x2,x2),
        dot(x3,x3)
    ), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4(
        dot(g0,x0),
        dot(g1,x1),
        dot(g2,x2),
        dot(g3,x3) ));
}

float fbm(vec3 p) {
    float f = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; i++) {
        f += amp * snoise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return f;
}

float getCloudDensity(vec3 pos) {
    vec3 movingPos = pos + vec3(time * speed, 0.0, time * speed * 0.5);
    float n = fbm(movingPos * 0.5);
    n = smoothstep(0.4, 0.7, n); // shape clouds
    return n;
}

void main()
{
    vec3 rayDir = normalize(farPoint - nearPoint);
    vec3 rayOrigin = nearPoint;

    float totalDensity = 0.0;

    for (float t = 0.0; t < 1.0; t += stepSize) {
        vec3 samplePos = rayOrigin + rayDir * t * cloudHeight;
        float d = getCloudDensity(samplePos);
        totalDensity += d * stepSize;
    }

    float light = dot(normalize(vec3(0.3, 0.6, 0.5)), rayDir);
    light = clamp(light, 0.0, 1.0);
    vec3 color = mix(vec3(0.8,0.8,0.9), vec3(1.0), light);

    FragColor = vec4(color, totalDensity);
}