// VERTEX SHADER
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;
out vec3 ViewDir;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = aPos;
    ViewDir = normalize(aPos);
    mat4 viewNoTranslation = mat4(mat3(view));
    gl_Position = projection * viewNoTranslation * vec4(aPos, 1.0);
}