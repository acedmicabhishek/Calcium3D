#version 330 core

// Output color
out vec4 FragColor;

// Input from vertex shader
in vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}