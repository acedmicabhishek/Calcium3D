#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;


// Imports the color from the Vertex Shader
in vec3 color;
// Imports the texture coordinates from the Vertex Shader
in vec2 texCoord;
// Imports the normal from the Vertex Shader
in vec3 Normal;
// Imports the current position from the Vertex Shader
in vec3 crntPos;

// Gets the Texture Unit from the main function
uniform sampler2D tex0;
uniform sampler2D tex1;
// Gets the color of the light from the main function
uniform vec4 lightColor;
// Gets the position of the light from the main function
uniform vec3 lightPos;
// Gets the position of the camera from the main function
uniform vec3 camPos;
// Sun (Global Light) uniforms
uniform vec4 sunColor;
uniform vec3 sunPos;
uniform float sunIntensity;

void main()
{
	// ambient lighting (reduced since sun will provide most lighting)
	float ambient = 0.10f;

	// diffuse lighting from sun (primary light source)
	vec3 normal = normalize(Normal);
	vec3 sunDirection = normalize(sunPos - crntPos);
	float sunDiffuse = max(dot(normal, sunDirection), 0.0f) * sunIntensity;

	// specular lighting from sun
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 sunReflectionDirection = reflect(-sunDirection, normal);
	float sunSpecAmount = pow(max(dot(viewDirection, sunReflectionDirection), 0.0f), 16);
	float sunSpecular = sunSpecAmount * specularLight * sunIntensity;

	// Calculate lighting primarily from sun
	vec4 sunLighting = (texture(tex0, texCoord) * (sunDiffuse + ambient) + texture(tex1, texCoord).r * sunSpecular) * sunColor;
	
	// outputs final color (sun is the main light source)
	FragColor = sunLighting;
}