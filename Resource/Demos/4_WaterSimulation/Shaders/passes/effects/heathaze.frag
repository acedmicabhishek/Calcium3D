#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 LocalPos;

uniform float time;
uniform float intensity; 

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }
float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i);
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

void main()
{
    float effIntensity = intensity;
    vec2 centeredUV = TexCoords - vec2(0.5);
    float dist = length(centeredUV); 
    
    if (dist > 0.5) discard;

    float edgeAlpha = smoothstep(0.5, 0.0, dist);
    
    vec2 noiseUV1 = TexCoords * vec2(4.0, 10.0) + vec2(0.0, -time * 2.0);
    float n1 = snoise(noiseUV1);
    
    vec2 noiseUV2 = TexCoords * vec2(8.0, 16.0) + vec2(time * 0.1, -time * 4.0);
    float n2 = snoise(noiseUV2);
    
    float finalNoise = (n1 * 0.6 + n2 * 0.4) * 0.5 + 0.5;

    vec3 coldColor = vec3(0.7, 0.8, 1.0);
    vec3 hotColor = vec3(1.0, 0.4, 0.1);
    vec3 shimmerColor = mix(coldColor, hotColor, finalNoise);
    
    float shimmerAlpha = (0.1 + finalNoise * 0.2) * edgeAlpha * effIntensity;
    
    float grain = fract(sin(dot(TexCoords, vec2(12.9898, 78.233))) * 43758.5453);
    if (shimmerAlpha < grain * 0.05) discard;

    FragColor = vec4(shimmerColor, shimmerAlpha);
}
