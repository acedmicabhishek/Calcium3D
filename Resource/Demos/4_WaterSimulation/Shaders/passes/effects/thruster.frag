#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 LocalPos;

uniform float time;
uniform float intensity; 

float hash(float n) { return fract(sin(n) * 43758.5453123); }

void main()
{

    float effIntensity = intensity;

    float radialDist = length(LocalPos.xy) * 2.0; 
    if (radialDist > 1.0) discard;

    float t = time * 30.0;
    float flicker = hash(floor(t)) * 0.2 + 0.9;
    
    float zPos = clamp(LocalPos.z + 0.5, 0.0, 1.0);
    
    float verticalMask = 1.0 - smoothstep(0.0, 1.0, zPos);
    
    float horizGlow = 1.0 - radialDist;
    horizGlow = pow(horizGlow, 2.0);


    vec3 coreColor = vec3(1.0, 1.0, 1.0);
    vec3 midColor  = vec3(1.0, 0.6, 0.1);
    vec3 edgeColor = vec3(1.0, 0.1, 0.0);
    
    float turb = hash(zPos * 10.0 + TexCoords.x * 5.0 - time * 20.0) * 0.1;
    float noiseGlow = horizGlow + turb * verticalMask;

    float coreFactor = pow(clamp(noiseGlow, 0.0, 1.0), 6.0) * (1.0 - zPos * 1.5);
    coreFactor = clamp(coreFactor, 0.0, 1.0);
    
    vec3 color = mix(midColor, coreColor, coreFactor);
    color = mix(edgeColor, color, noiseGlow);
    
    float alpha = verticalMask * noiseGlow * effIntensity * flicker;
    
    vec3 finalColor = color * (1.5 + coreFactor * 4.0) * (0.5 + effIntensity * 0.5);

    FragColor = vec4(finalColor, alpha); 
}
