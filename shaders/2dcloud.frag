#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform sampler2D noiseTex;
in vec2 TexCoords;

void main()
{
    float noise = texture(noiseTex, TexCoords).r;
    float alpha = smoothstep(0.4, 0.6, noise);

    if (alpha < 0.1)
        discard;

    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}