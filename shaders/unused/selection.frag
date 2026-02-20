#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Inputs from vertex shader
in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

// Texture uniforms
uniform sampler2D tex0;
uniform sampler2D tex1;

// Sun lighting uniforms
uniform vec4 sunColor;
uniform vec3 sunPos;
uniform float sunIntensity;

// Camera position
uniform vec3 camPos;

void main()
{
	// Set selected object to bright red
	vec3 selectionColor = vec3(1.0, 0.0, 0.0); // Bright red
	
	// Calculate lighting for the red color
	vec3 normal = normalize(Normal);
	vec3 sunDirection = normalize(sunPos - crntPos);
	float sunDiffuse = max(dot(normal, sunDirection), 0.0f) * sunIntensity;
	
	// Specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - crntPos);
	vec3 sunReflectionDirection = reflect(-sunDirection, normal);
	float sunSpecAmount = pow(max(dot(viewDirection, sunReflectionDirection), 0.0f), 16);
	float sunSpecular = sunSpecAmount * specularLight * sunIntensity;
	
	// Ambient lighting
	float ambient = 0.3f;
	
	// Apply lighting to the red color
	vec3 finalColor = selectionColor * (sunDiffuse + ambient) + selectionColor * sunSpecular * 0.5;
	
	// Output bright red with lighting
	FragColor = vec4(finalColor, 1.0);
}

