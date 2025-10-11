#include <glad/glad.h>
#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp> 
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint ID;

    
    Shader(const char* vertexPath, const char* fragmentPath);
    
    void use();
    void Delete();
    
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;

private:
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif