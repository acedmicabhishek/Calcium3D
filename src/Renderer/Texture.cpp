#include "Texture.h"
#include <iostream>
#include <stdexcept>

#include "Core/ResourceManager.h"
#include <filesystem>

Texture::Texture(const char *image, const char *texType, GLuint slot) {
  type = texType;
  unit = slot;
  path = image;

  int widthImg, heightImg, numColCh;
  stbi_set_flip_vertically_on_load(true);

  std::string resolved = ResourceManager::ResolvePath(image);
  unsigned char *bytes =
      stbi_load(resolved.c_str(), &widthImg, &heightImg, &numColCh, 0);
  if (!bytes) {
    std::cout << "Failed to load texture: " << resolved << std::endl;
    return;
  }

  glGenTextures(1, &ID);

  glActiveTexture(GL_TEXTURE0 + slot);
  unit = slot;
  glBindTexture(GL_TEXTURE_2D, ID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if (type == "specular") {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widthImg, heightImg, 0, GL_RED,
                 GL_UNSIGNED_BYTE, bytes);
  } else if (numColCh == 4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, bytes);
  } else if (numColCh == 3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthImg, heightImg, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, bytes);
  } else if (numColCh == 1) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widthImg, heightImg, 0, GL_RED,
                 GL_UNSIGNED_BYTE, bytes);
    if (std::string(type) != "specular") {
      GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
  } else if (numColCh == 2) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, widthImg, heightImg, 0, GL_RG,
                 GL_UNSIGNED_BYTE, bytes);
    if (std::string(type) != "specular") {
      GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
  } else {
    std::cout << "Warning: Unsupported channel count (" << numColCh
              << ") for texture: " << image << ". Using fallback." << std::endl;
    unsigned char white[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 white);
  }

  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(bytes);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader &shader, const char *uniform, GLuint unit) const {
  GLuint texUni = glGetUniformLocation(shader.ID, uniform);
  shader.use();
  glUniform1i(texUni, unit);
}

void Texture::Bind() const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void Texture::Delete() { glDeleteTextures(1, &ID); }