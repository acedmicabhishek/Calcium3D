#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include<glad/glad.h>
#include<vector>

class EBO
{
public:
	
	GLuint ID;
	
	EBO(const std::vector<GLuint>& indices);

	
	void Bind();
	
	void Unbind();
	
	void Delete();
};

#endif