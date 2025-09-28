#version 330 core
out vec4 FragColor;

in vec3 FragPos;

in vec2 TexCoords;
in vec3 WorldPos;

// 2D simplex noise
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m;
    m = m*m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

uniform float u_time;
uniform vec2 u_resolution;
uniform vec3 u_cloudColor;
uniform float u_cloudCover;
uniform float u_cloudSpeed;
uniform float u_tiling;
uniform float u_density;
uniform float u_cloudSize;

float fbm(vec2 p) {
    float f = 0.0;
    f += 0.5000 * snoise(p); p = p * 2.02;
    f += 0.2500 * snoise(p); p = p * 2.03;
    f += 0.1250 * snoise(p); p = p * 2.01;
    f += 0.0625 * snoise(p);
    return f / 0.9375;
}

void main()
{
    vec2 uv = WorldPos.xz * 0.01 * u_tiling; // Use world position for noise generation
    vec2 motion = vec2(u_time * u_cloudSpeed * 0.1, 0.0);

    // Base cloud shape
    float base_noise = fbm(uv * u_cloudSize + motion);
    float cloud_shape = smoothstep(1.0 - u_cloudCover, 1.0, base_noise);

    // Detail noise to create more complex edges
    float detail_noise = fbm(uv * 8.0 + motion * 2.0);
    cloud_shape *= smoothstep(0.4, 0.6, detail_noise);

    // Final alpha, modulated by density
    float alpha = cloud_shape * u_density;

    if (alpha < 0.01)
        discard;

    // Simple lighting - brighten clouds based on noise value (thicker parts are brighter)
    vec3 final_color = u_cloudColor * (1.0 + base_noise * 0.2);

    FragColor = vec4(final_color, alpha);
}