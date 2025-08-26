#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include<glad/glad.h>
#include<stb/stb_image.h>

#include"Shader.h"

class Texture
{
public:
	GLuint ID;
	const char* type;
	GLuint unit;

	Texture(const char* image = "../Resource/default/texture/DefaultTex.png", const char* texType = "diffuse", GLuint slot = 0);

	
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	
	void Bind();
	
	void Unbind();
	
	void Delete();
};
#endif