#version 330 core
out vec4 FragColor;

in vec3 WorldNormal;
in vec3 WorldPos;

uniform vec3 cullingCameraPos;
uniform vec3 color;
uniform bool isBackfacePass;

void main() {
    vec3 viewDir = normalize(WorldPos - cullingCameraPos);
    
    float dotProduct = dot(WorldNormal, viewDir);
    
    bool isBackface = dotProduct > 0.0;
    
    if (isBackfacePass) {
        if (!isBackface) discard;
    } else {
        if (isBackface) discard;
    }
    
    FragColor = vec4(color, 1.0);
}
