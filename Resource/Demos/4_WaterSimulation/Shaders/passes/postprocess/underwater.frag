#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform float time;
uniform vec3 waterColor;
uniform mat4 invView;
uniform mat4 invProjection;
uniform vec3 camPos;
uniform float waterLevel;

uniform mat4 invModel;
uniform vec3 localBoxMin;
uniform vec3 localBoxMax;
uniform float liquidDensity;

vec3 worldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = invProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = invView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

vec2 rayBoxDist(vec3 ro, vec3 rd, vec3 boxMin, vec3 boxMax) {
    vec3 t0 = (boxMin - ro) / rd;
    vec3 t1 = (boxMax - ro) / rd;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    float distA = max(max(tmin.x, tmin.y), tmin.z);
    float distB = min(min(tmax.x, tmax.y), tmax.z);
    return vec2(distA, distB);
}

void main()
{
    vec2 distortedCoords = TexCoords;
    distortedCoords.x += sin(TexCoords.y * 10.0 + time * 2.0) * 0.005;
    distortedCoords.y += cos(TexCoords.x * 10.0 + time * 2.0) * 0.005;

    vec3 sceneColor = texture(screenTexture, distortedCoords).rgb;
    float depth = texture(depthTexture, distortedCoords).r;
    
    vec3 worldPos = worldPosFromDepth(depth);
    float hitDist = length(worldPos - camPos);
    
    vec3 localCam = (invModel * vec4(camPos, 1.0)).xyz;
    vec3 localWorld = (invModel * vec4(worldPos, 1.0)).xyz;
    vec3 localDir = normalize(localWorld - localCam);
    
    vec2 boxIntersections = rayBoxDist(localCam, localDir, localBoxMin, localBoxMax);
    float t_near = max(0.0, boxIntersections.x);
    float t_far  = boxIntersections.y;
    
    float localHitDist = length(localWorld - localCam);
    float waterThicknessLocal = max(0.0, min(localHitDist, t_far));
    
    float worldThickness = waterThicknessLocal * length(vec3(1.0/invModel[0][0], 1.0/invModel[1][1], 1.0/invModel[2][2])) / 1.732;

    float liquidPath = min(hitDist, t_far * (hitDist/localHitDist)); 

    float fogFactor = clamp(liquidPath * 0.05 * liquidDensity, 0.0, 1.0);
    vec3 finalColor = mix(sceneColor, waterColor * 0.6, fogFactor);
    
    finalColor *= vec3(0.8, 0.9, 1.2);
    
    float depthFromSurface = waterLevel - worldPos.y;
    if (depthFromSurface > 0.0 && liquidPath > 0.0) {
        float causticScale = 0.5;
        vec2 p = worldPos.xz * causticScale;
        float c = sin(p.x + time) + sin(p.y + time) + sin(p.x - p.y + time * 0.5);
        c = pow(clamp(c * 0.5 + 0.5, 0.0, 1.0), 4.0);
        
        float causticFade = clamp(1.0 - depthFromSurface * 0.2, 0.0, 1.0);
        finalColor += vec3(0.5, 0.7, 1.0) * c * causticFade * 0.4;
    }

    FragColor = vec4(finalColor, 1.0);
}
