#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform sampler2D noiseTex;
uniform vec3 viewPos;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 rayDir = normalize(FragPos - viewPos);
    vec3 rayOrigin = viewPos;

    float maxDist = 200.0;
    float stepSize = 2.0;
    float totalDensity = 0.0;
    vec3 lightColor = vec3(1.0, 0.9, 0.8);

    for(float d = 0.0; d < maxDist; d += stepSize)
    {
        vec3 p = rayOrigin + rayDir * d;
        float noise = texture(noiseTex, p.xz * 0.01).r;
        float density = smoothstep(0.4, 0.6, noise);
        
        if(density > 0.0)
        {
            totalDensity += density * stepSize;
        }
    }

    vec3 cloudColor = vec3(1.0, 1.0, 1.0) * totalDensity * 0.1;
    FragColor = vec4(cloudColor, smoothstep(0.0, 1.0, totalDensity));
}