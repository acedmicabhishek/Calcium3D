#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 weights;

out vec3 color;
out vec2 texCoord;
out vec3 Normal;
out vec3 crntPos;
out vec4 FragPosLightSpace;

uniform mat4 camMatrix;
uniform mat4 model;
uniform vec3 tilingFactor;
uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    
    bool hasBones = false;
    for(int i = 0 ; i < 4 ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
            break;

        hasBones = true;
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * weights[i];
        
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
        totalNormal += localNormal * weights[i];
    }
    
    if (!hasBones) {
        totalPosition = vec4(aPos, 1.0f);
        totalNormal = aNormal;
    }

	crntPos = vec3(model * totalPosition);
	gl_Position = camMatrix * vec4(crntPos, 1.0);
	FragPosLightSpace = lightSpaceMatrix * vec4(crntPos, 1.0);

	color = aColor;
	vec3 n = abs(totalNormal);
	n = n / (n.x + n.y + n.z + 1e-6);
	vec2 uvScale = vec2(tilingFactor.z, tilingFactor.y) * n.x +
	               vec2(tilingFactor.x, tilingFactor.z) * n.y +
	               vec2(tilingFactor.x, tilingFactor.y) * n.z;
	               
	texCoord = aTex * uvScale;
	Normal = mat3(transpose(inverse(model))) * totalNormal;
}
