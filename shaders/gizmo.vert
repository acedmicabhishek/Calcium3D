#version 330 core

// Position attribute
layout (location = 0) in vec3 aPos;
// Color attribute  
layout (location = 1) in vec3 aColor;

// Outputs
out vec3 color;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate final position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Pass color to fragment shader
    color = aColor;
}