#version 330 core
out vec4 FragColor;

uniform vec2 iResolution;
uniform float iTime;

const int MAX_ITERATIONS = 256;

vec3 palette(float t) {
    return 0.5 + 0.5 * cos(6.28318 * (vec3(0.8, 0.5, 0.4) * t + vec3(0.0, 0.15, 0.20)));
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;

    // Infinite zoom
    float zoomSpeed = 0.3; // zoom speed
    float zoom = exp(iTime * zoomSpeed);
    
    // Target point in the "steam" - a thin filament with infinite detail
    vec2 target = vec2(-0.74529, 0.11307); // A point known for deep zoom potential
    
    // Add slight rotation for more dynamic effect
    float angle = iTime * 0.02;
    float cosA = cos(angle);
    float sinA = sin(angle);
    
    vec2 c = uv / zoom;
    // Apply rotation
    c = vec2(c.x * cosA - c.y * sinA, c.x * sinA + c.y * cosA);
    c += target;

    vec2 z = vec2(0.0);
    float iter = 0.0;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 4.0) {
            iter = float(i);
            break;
        }
    }

    vec3 color = vec3(0.0);
    if (iter > 0.0) {
        float colorShift = log(zoom) * 0.1 + iTime * 0.1;
        color = palette(iter / float(MAX_ITERATIONS) + colorShift);
    }

    FragColor = vec4(color, 1.0);
}