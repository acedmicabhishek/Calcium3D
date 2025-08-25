#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 aColor;
// Texture Coordinates
layout (location = 2) in vec2 aTex;
// Normals
layout (location = 3) in vec3 aNormal;

// Outputs
out vec3 color;
out vec2 texCoord;
out vec3 Normal;
out vec3 crntPos;

// Uniforms
uniform mat4 camMatrix;
uniform mat4 model;
uniform vec3 tilingFactor;

void main()
{
	// Calculate current position
	crntPos = vec3(model * vec4(aPos, 1.0f));
	
	// Outputs
	gl_Position = camMatrix * vec4(crntPos, 1.0);
	color = aColor;
	texCoord = aTex * tilingFactor.xy;
	Normal = mat3(transpose(inverse(model))) * aNormal;
}
