#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 nearPoint;
out vec3 farPoint;

uniform mat4 invProjection;
uniform mat4 invView;

void main()
{
    vec4 near = invProjection * vec4(aPos.xy, -1.0, 1.0);
    nearPoint = (invView * vec4(near.xyz / near.w, 1.0)).xyz;

    vec4 far = invProjection * vec4(aPos.xy, 1.0, 1.0);
    farPoint = (invView * vec4(far.xyz / far.w, 1.0)).xyz;

    gl_Position = vec4(aPos, 1.0);
}