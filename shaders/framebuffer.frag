#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{
	vec3 color = texture(screenTexture, texCoords).rgb;
	float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
	FragColor = vec4(average, average, average, 1.0);
}