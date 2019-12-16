#include <QDebug>

#include "Material.h"

Material::Material() :
  albedo(glm::vec3(1.0f)),
  ambient(glm::vec3(0.2f)),
  specularity(1.0f),
  shininess(64.0f)
{
  initializeOpenGLFunctions();
}

Material::~Material() {}

void Material::set_materials(Shader *shader) {
  shader->use();
  shader->setFloat("material.metalness", metalness);

  shader->setInt("material.number_albedo_maps", albedo_maps.size());
  shader->setBool("material.use_specular_map", specular_map.id!=0);
  shader->setBool("material.use_ambient_occlusion_map", false);

  for (unsigned int i=0; i<albedo_maps.size(); i++) {
    glActiveTexture(GL_TEXTURE0+i);
    shader->setInt(("material.albedo_map["+std::to_string(i)+"]").c_str(), i);
    glBindTexture(GL_TEXTURE_2D, albedo_maps[i].id);
  }
  if (specular_map.id != 0) {
    glActiveTexture(GL_TEXTURE0+albedo_maps.size());
    shader->setInt("material.specular_map",albedo_maps.size());
    glBindTexture(GL_TEXTURE_2D, specular_map.id);
  }

  shader->setVec3("material.albedo", albedo);
  shader->setVec3("material.ambient", ambient);
  shader->setFloat("material.specularity", specularity);

  shader->setFloat("material.shininess", shininess);
}

void Material::load_texture(const char *path, Image_Type type) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  QImage img = QImage(path).convertToFormat(QImage::Format_RGB888);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
  glGenerateMipmap(GL_TEXTURE_2D);

  switch (type) {
    case ALBEDO_MAP:
      albedo_maps.push_back(Texture({textureID, "albedo_map", path}));
      break;
    case AMBIENT_OCCLUSION_MAP:
      ambient_occlusion_map = Texture({textureID, "ambient_occlusion_map", path});
      break;
    case SPECULAR_MAP:
      specular_map = Texture({textureID, "specular_map", path});
      break;
  }
}
