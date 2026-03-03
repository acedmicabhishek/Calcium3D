#include "ObjectFactory.h"
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

Mesh ObjectFactory::createCube() {
    Vertex vertices[] =
    { 
        
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        
        Vertex{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)}
    };

    GLuint indices[] =
    {
        0, 1, 2, 0, 2, 3,       
        4, 5, 6, 4, 6, 7,       
        8, 9, 10, 8, 10, 11,    
        12, 13, 14, 12, 14, 15, 
        16, 17, 18, 16, 18, 19, 
        20, 21, 22, 20, 22, 23  
    };

    std::vector<Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    std::vector<Texture> tex;
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);

    return Mesh(verts, ind, tex);

}

Mesh ObjectFactory::createPlane() {
    Vertex vertices[] =
    {
        Vertex{glm::vec3(-5.0f, 0.0f,  5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3( 5.0f, 0.0f,  5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3( 5.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)}
    };

    GLuint indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    std::vector<Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
    std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
    std::vector<Texture> tex;
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);

    return Mesh(verts, ind, tex);
}

Mesh ObjectFactory::createSphere(int sectorCount, int stackCount) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / 0.5f;
    float s, t;

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = 0.5f * cosf(stackAngle);
        z = 0.5f * sinf(stackAngle);

        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            vertices.push_back(Vertex{glm::vec3(x, y, z), glm::vec3(nx, ny, nz), glm::vec3(1.0f), glm::vec2(s, t)});
        }
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    std::vector<Texture> tex;
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);
    return Mesh(vertices, indices, tex);
}

Mesh ObjectFactory::createCameraMesh() {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    
    auto addBox = [&](glm::vec3 minP, glm::vec3 maxP, glm::vec3 color) {
        int base = vertices.size();
        glm::vec3 c[8] = {
            {minP.x, minP.y, maxP.z}, {maxP.x, minP.y, maxP.z}, {maxP.x, maxP.y, maxP.z}, {minP.x, maxP.y, maxP.z},
            {maxP.x, minP.y, minP.z}, {minP.x, minP.y, minP.z}, {minP.x, maxP.y, minP.z}, {maxP.x, maxP.y, minP.z}
        };
        
        vertices.insert(vertices.end(), { {c[0], {0,0,1}, color, {0,0}}, {c[1], {0,0,1}, color, {1,0}}, {c[2], {0,0,1}, color, {1,1}}, {c[3], {0,0,1}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;
        
        vertices.insert(vertices.end(), { {c[4], {0,0,-1}, color, {0,0}}, {c[5], {0,0,-1}, color, {1,0}}, {c[6], {0,0,-1}, color, {1,1}}, {c[7], {0,0,-1}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;
        
        vertices.insert(vertices.end(), { {c[5], {-1,0,0}, color, {0,0}}, {c[0], {-1,0,0}, color, {1,0}}, {c[3], {-1,0,0}, color, {1,1}}, {c[6], {-1,0,0}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;

        vertices.insert(vertices.end(), { {c[1], {1,0,0}, color, {0,0}}, {c[4], {1,0,0}, color, {1,0}}, {c[7], {1,0,0}, color, {1,1}}, {c[2], {1,0,0}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;

        vertices.insert(vertices.end(), { {c[3], {0,1,0}, color, {0,0}}, {c[2], {0,1,0}, color, {1,0}}, {c[7], {0,1,0}, color, {1,1}}, {c[6], {0,1,0}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;

        vertices.insert(vertices.end(), { {c[5], {0,-1,0}, color, {0,0}}, {c[4], {0,-1,0}, color, {1,0}}, {c[1], {0,-1,0}, color, {1,1}}, {c[0], {0,-1,0}, color, {0,1}} });
        indices.insert(indices.end(), { (GLuint)base, (GLuint)(base+1), (GLuint)(base+2), (GLuint)base, (GLuint)(base+2), (GLuint)(base+3) }); base += 4;
    };

    addBox({-0.15f, -0.1f, -0.1f}, {0.15f, 0.1f, 0.15f}, {0.3f, 0.3f, 0.3f}); 
    addBox({-0.08f, -0.08f, -0.2f}, {0.08f, 0.08f, -0.1f}, {0.1f, 0.1f, 0.1f}); 
    addBox({-0.08f, 0.1f, -0.05f}, {0.08f, 0.15f, 0.05f}, {0.2f, 0.2f, 0.2f}); 
    
    std::vector<Texture> tex;
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    tex.emplace_back("../Resource/default/texture/DefaultTex.png", "specular", 1);
    return Mesh(vertices, indices, tex);
}
