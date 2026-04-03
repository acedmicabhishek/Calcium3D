#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform samplerCube skyboxCubemap;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 invProjection;
uniform mat4 invView;
uniform vec3 camPos;

uniform float ssrResolution;
uniform int   ssrMaxSteps;
uniform float ssrMaxDistance;
uniform float ssrThickness;
uniform float ssrRenderDistance;
uniform float ssrFadeStart;
uniform int   ssrUseCubemapFallback;
uniform int   reflectionMode;
uniform int   isAllPass;

vec3 getViewPos(vec2 uv) {
    float d  = texture(depthTexture, uv).r;
    vec4  ndc = vec4(uv * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);
    vec4  vp  = invProjection * ndc;
    return vp.xyz / vp.w;
}

float edgeFade(vec2 uv) {
    const float margin = 0.1;
    vec2 f = smoothstep(0.0, margin, uv) * (1.0 - smoothstep(1.0 - margin, 1.0, uv));
    return clamp(f.x * f.y, 0.0, 1.0);
}

void main() {
    float myDepth = texture(depthTexture, TexCoords).r;
    if (myDepth >= 1.0) discard;

    vec3 viewPos    = getViewPos(TexCoords);
    vec4  nMap    = texture(normalTexture, TexCoords);
    vec3  worldN  = nMap.xyz;
    float metallic = nMap.a;

    if (dot(worldN, worldN) < 0.1 || metallic < 0.01) discard;

    if (reflectionMode == 2) {
        vec3 viewNrm   = normalize(mat3(view) * normalize(worldN));
        vec3 vDir      = normalize(viewPos);
        vec3 refv      = normalize(reflect(vDir, viewNrm));
        vec3 worldRef  = mat3(invView) * refv;
        vec3 color     = texture(skyboxCubemap, worldRef).rgb;
        FragColor = vec4(color, metallic);
        return;
    }

    float fragDist = length(viewPos);
    if (fragDist > ssrRenderDistance) discard;
    float distanceFade = 1.0 - smoothstep(ssrFadeStart, ssrRenderDistance, fragDist);
    float effectiveMetal = metallic * distanceFade;

    vec3 viewNormal = normalize(mat3(view) * normalize(worldN));
    vec3 viewDir    = normalize(viewPos);
    vec3 reflectDir = normalize(reflect(viewDir, viewNormal));

    if (reflectDir.z > 0.95) discard;

    if (isAllPass == 1) {
        vec3 worldReflect = mat3(invView) * reflectDir;
        if (worldReflect.y > 0.0) {
            vec4 farPoint = projection * view * vec4(camPos + worldReflect * 1000.0, 1.0);
            vec2 skyUV = (farPoint.xy / farPoint.w) * 0.5 + 0.5;
            float skyAlpha = effectiveMetal * edgeFade(skyUV) * 0.55;
            if (skyAlpha > 0.001) {
                vec3 skyColor;
                if (skyUV.x >= 0.0 && skyUV.x <= 1.0 && skyUV.y >= 0.0 && skyUV.y <= 1.0) {
                    skyColor = texture(colorTexture, skyUV).rgb;
                } else {
                    skyColor = texture(skyboxCubemap, worldReflect).rgb;
                }
                FragColor = vec4(skyColor, skyAlpha);
                return;
            }
        }
    }

    float startBias = ssrThickness * 0.5 + 0.05;
    vec3  ray       = viewPos + viewNormal * startBias;

    float stepSize = ssrMaxDistance / float(ssrMaxSteps);

    vec3  hitColor = vec3(0.0);
    float hitMask  = 0.0;

    for (int i = 0; i < ssrMaxSteps; ++i) {
        ray += reflectDir * stepSize;

        vec4 proj = projection * vec4(ray, 1.0);
        if (proj.w <= 0.0) break;
        proj.xyz /= proj.w;

        vec2 uv = proj.xy * 0.5 + 0.5;

        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
            if (ssrUseCubemapFallback == 1) {
                vec3 worldReflect = mat3(invView) * reflectDir;
                hitColor = texture(skyboxCubemap, worldReflect).rgb;
                float dist = length(ray - (viewPos + viewNormal * startBias));
                float dFac  = 1.0 - clamp(dist / ssrMaxDistance, 0.0, 1.0);
                hitMask = dFac * 0.4 * effectiveMetal;
            }
            break;
        }
        if (proj.z < -1.0 || proj.z > 1.0) break;

        float sampledD = texture(depthTexture, uv).r;

        if (sampledD >= 1.0) {
            if (ssrUseCubemapFallback == 1) {
                vec3 worldReflect = mat3(invView) * reflectDir;
                vec3 cubemapSky   = texture(skyboxCubemap, worldReflect).rgb;
                vec3 screenSky    = texture(colorTexture, uv).rgb;
                hitColor = mix(cubemapSky, screenSky, 0.6);
            } else {
                hitColor = texture(colorTexture, uv).rgb;
            }
            float dist = length(ray - (viewPos + viewNormal * startBias));
            float dFac = 1.0 - clamp(dist / ssrMaxDistance, 0.0, 1.0);
            hitMask = dFac * 0.45 * effectiveMetal;
            break;
        }

        vec3 scenePos  = getViewPos(uv);
        float penDepth = scenePos.z - ray.z;

        if (penDepth > 0.0 && penDepth < ssrThickness) {
            vec3 lo = ray - reflectDir * stepSize;
            vec3 hi = ray;
            for (int b = 0; b < 5; ++b) {
                vec3 mid = (lo + hi) * 0.5;
                vec4 mp  = projection * vec4(mid, 1.0);
                mp.xyz  /= mp.w;
                vec2 muv = mp.xy * 0.5 + 0.5;
                vec3 sp  = getViewPos(muv);
                float p  = sp.z - mid.z;
                if (p > 0.0 && p < ssrThickness * 2.0)
                    hi = mid;
                else
                    lo = mid;
            }

            vec4 fp  = projection * vec4((lo + hi) * 0.5, 1.0);
            fp.xyz  /= fp.w;
            vec2 fUV = fp.xy * 0.5 + 0.5;
            if (fUV.x < 0.0 || fUV.x > 1.0 || fUV.y < 0.0 || fUV.y > 1.0) break;

            hitColor = texture(colorTexture, fUV).rgb;
            float edge = edgeFade(fUV);
            float dist = length(ray - (viewPos + viewNormal * startBias));
            float dFac = 1.0 - clamp(dist / ssrMaxDistance, 0.0, 1.0);
            hitMask = edge * dFac * effectiveMetal;
            break;
        }
    }

    if (hitMask < 0.001) discard;
    FragColor = vec4(hitColor, hitMask);
}
