#include"Texture.h"
#include<iostream>
#include<stdexcept>

#include <filesystem>

Texture::Texture(const char* image, const char* texType, GLuint slot)
{
	type = texType;
    unit = slot; 

	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
    
	
	
	
	unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);
	if (!bytes)
	{
		std::cout << "Failed to load texture: " << image << std::endl;
		return;
	}
	else
	{
		std::cout << "Successfully loaded texture: " << image << " (" << widthImg << "x" << heightImg << ", " << numColCh << " channels)" << std::endl;
	}
    

	
	glGenTextures(1, &ID);
	
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;
	glBindTexture(GL_TEXTURE_2D, ID);

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	
	
	

	
	if (type == "specular")
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widthImg, heightImg, 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColCh == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColCh == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColCh == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widthImg, heightImg, 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
	}
	else
	{
		throw std::invalid_argument("Automatic texture type recognition failed");
	}
	
	glGenerateMipmap(GL_TEXTURE_2D);

	
	stbi_image_free(bytes);

	
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	
	shader.use(); 
	
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}