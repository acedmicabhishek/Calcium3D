#version 330 core

// Output color
out vec4 FragColor;

// Input from vertex shader
in vec3 color;

void main()
{
    // Output the gizmo color directly
    FragColor = vec4(color, 1.0);
}