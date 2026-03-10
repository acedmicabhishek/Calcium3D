#include "TextureAtlas.h"
#include "Core/Logger.h"
#include <algorithm>
#include <glad/glad.h>
#include <stb/stb_image.h>

TextureAtlas::TextureAtlas(int width, int height)
    : m_Width(width), m_Height(height), m_AtlasID(0) {
  m_Shelves.push_back({0, 0, 0});
}

TextureAtlas::~TextureAtlas() {
  if (m_AtlasID)
    glDeleteTextures(1, &m_AtlasID);
}

bool TextureAtlas::AddTexture(const std::string &path) {
  int w, h, c;
  unsigned char *data = stbi_load(path.c_str(), &w, &h, &c, 4);
  if (!data) {
    Logger::AddLog("[Atlas] Failed to load %s", path.c_str());
    return false;
  }
  m_PendingTextures.push_back({path, w, h, 4, data});
  return true;
}

void TextureAtlas::Bake() {
  if (m_PendingTextures.empty())
    return;

  
  std::sort(m_PendingTextures.begin(), m_PendingTextures.end(),
            [](const RawTexture &a, const RawTexture &b) { return a.h > b.h; });

  unsigned char *atlasData = (unsigned char *)calloc(m_Width * m_Height * 4, 1);

  int shelfX = 0;
  int shelfY = 0;
  int shelfH = 0;

  for (auto &tex : m_PendingTextures) {
    if (shelfX + tex.w > m_Width) {
      shelfY += shelfH;
      shelfX = 0;
      shelfH = 0;
    }

    if (shelfY + tex.h > m_Height) {
      Logger::AddLog("[Atlas] Out of space for %s", tex.path.c_str());
      stbi_image_free(tex.data);
      continue;
    }

    
    for (int row = 0; row < tex.h; ++row) {
      memcpy(atlasData + ((shelfY + row) * m_Width + shelfX) * 4,
             tex.data + (row * tex.w) * 4, tex.w * 4);
    }

    AtlasEntry entry;
    entry.offset = glm::vec2((float)shelfX / m_Width, (float)shelfY / m_Height);
    entry.size = glm::vec2((float)tex.w / m_Width, (float)tex.h / m_Height);
    m_Entries[tex.path] = entry;

    shelfX += tex.w;
    shelfH = std::max(shelfH, tex.h);

    stbi_image_free(tex.data);
  }
  m_PendingTextures.clear();

  glGenTextures(1, &m_AtlasID);
  glBindTexture(GL_TEXTURE_2D, m_AtlasID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, atlasData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(atlasData);
  Logger::AddLog("[Atlas] Baked %dx%d atlas with %d textures.", m_Width,
                 m_Height, (int)m_Entries.size());
}

AtlasEntry TextureAtlas::GetEntry(const std::string &path) const {
  if (m_Entries.count(path))
    return m_Entries.at(path);
  return {glm::vec2(0.0f), glm::vec2(1.0f)};
}
