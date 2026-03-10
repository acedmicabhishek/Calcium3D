#include <glad/glad.h>
#ifndef SHADER_H
#define SHADER_H

#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

class Shader {
public:
  GLuint ID;

  Shader(const char *vertexPath, const char *fragmentPath,
         const char *geometryPath = nullptr);

  void use();
  void Delete();

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
  void setMat4(const std::string &name, const glm::mat4 &mat) const;
  void setVec2(const std::string &name, const glm::vec2 &value) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setVec4(const std::string &name, const glm::vec4 &value) const;

private:
  void checkCompileErrors(GLuint shader, std::string type);
  GLint GetUniformLocation(const std::string &name) const;
  mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

#endif