#version 330 core
out vec4 FragColor;

in vec3 WorldNormal;
in vec3 WorldPos;

uniform vec3 cullingCameraPos;
uniform vec3 color;
uniform bool isBackfacePass;

uniform bool isFrustumCulled;
uniform int cullingMode;

void main() {
    vec3 viewDir = normalize(cullingCameraPos - WorldPos);
    float dotProduct = dot(normalize(WorldNormal), viewDir);
    bool isBackface = (dotProduct <= 0.0);
    
    vec3 finalColor;
    if (isFrustumCulled) {
        finalColor = vec3(1.0, 0.6, 0.0);
    } else {
        if (isBackface) {
            finalColor = vec4(1.0, 0.0, 0.0, 1.0).rgb;
        } else {
            finalColor = vec4(0.0, 1.0, 0.0, 1.0).rgb;
        }
    }

    if (cullingMode == 1) {
        if (!isBackface) discard;
        FragColor = vec4(finalColor, 1.0);
    } else {
        if (isBackface) discard;
        FragColor = vec4(finalColor, 1.0);
    }
}
