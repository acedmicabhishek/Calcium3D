#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 aColor;
// Texture Coordinates
layout (location = 2) in vec2 aTex;
// Normals (not necessarily normalized)
layout (location = 3) in vec3 aNormal;


// Outputs the color for the Fragment Shader
out vec3 color;
// Outputs the texture coordinates to the Fragment Shader
out vec2 texCoord;
// Outputs the normal for the Fragment Shader
out vec3 Normal;
// Outputs the current position for the Fragment Shader
out vec3 crntPos;
// Outputs position in light space for directional shadows
out vec4 FragPosLightSpace;

// Imports the camera matrix from the main function
uniform mat4 camMatrix;
// Imports the model matrix from the main function
uniform mat4 model;
// Texture tiling factor
uniform vec3 tilingFactor;

// Per-object texture scaling
uniform bool textureScaling;
uniform float textureScaleValue;

// Shadow matrix
uniform mat4 lightSpaceMatrix;

void main()
{
	// calculates current position
	crntPos = vec3(model * vec4(aPos, 1.0f));
	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * vec4(crntPos, 1.0);
	FragPosLightSpace = lightSpaceMatrix * vec4(crntPos, 1.0);

	// Assigns the colors from the Vertex Data to "color"
	color = aColor;

	if (textureScaling) {
		// Texture scaling mode: stretch texture to fit, no tiling from object scale
		texCoord = aTex * textureScaleValue;
	} else {
		// Default tiling mode: tri-axis blending for correct scaling on all faces
		vec3 n = abs(aNormal);
		n = n / (n.x + n.y + n.z + 1e-6); // Avoid division by zero
		vec2 uvScale = vec2(tilingFactor.z, tilingFactor.y) * n.x +
		               vec2(tilingFactor.x, tilingFactor.z) * n.y +
		               vec2(tilingFactor.x, tilingFactor.y) * n.z;
		texCoord = aTex * uvScale;
	}

	// Transform normal to world space (handles non-uniform scale)
	Normal = mat3(transpose(inverse(model))) * aNormal;
}