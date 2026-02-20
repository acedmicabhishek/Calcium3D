#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTex;

out vec3 crntPos;
out vec3 Normal;
out vec3 color;
out vec2 texCoord;

uniform mat4 camMatrix;
uniform mat4 model;
uniform vec3 tilingFactor;

void main()
{
	crntPos = vec3(model * vec4(aPos, 1.0f));
	color = aColor;
	Normal = aNormal;
	// Tri-axis blending for correct scaling on all faces
	vec3 n = abs(aNormal);
	n = n / (n.x + n.y + n.z + 1e-6); 
	vec2 uvScale = vec2(tilingFactor.z, tilingFactor.y) * n.x +
	               vec2(tilingFactor.x, tilingFactor.z) * n.y +
	               vec2(tilingFactor.x, tilingFactor.y) * n.z;
	               
	texCoord = aTex * uvScale;

	gl_Position = camMatrix * vec4(crntPos, 1.0f);
}
