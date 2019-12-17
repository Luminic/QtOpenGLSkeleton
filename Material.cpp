#include <QDebug>

#include "Material.h"
#include "Scene.h"

Material::Material() :
  metalness(0.0f),
  ambient_occlusion_map({0,"",""}),
  specular_map({0,"",""}),
  albedo(glm::vec3(1.0f)),
  ambient(glm::vec3(0.2f)),
  specularity(1.0f),
  shininess(64.0f)
{
  initializeOpenGLFunctions();
}

Material::~Material() {
}

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
  Texture texture = Scene::is_texture_loaded(path);

  if (texture.id == 0) {
    texture.path = path;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    QImage img = QImage(path).convertToFormat(QImage::Format_RGB888);
    if (type == ALBEDO_MAP) { // Convert gamma (SRGB) space into linear (RGB) space
      glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    } else { // Non-albedo maps should already be in linear space
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    Scene::loaded_textures.push_back(texture);
  }

  switch (type) {
    case ALBEDO_MAP:
      texture.type = "albedo_map";
      albedo_maps.push_back(texture);
      break;
    case AMBIENT_OCCLUSION_MAP:
      texture.type = "ambient_occlusion_map";
      ambient_occlusion_map = texture;
      break;
    case SPECULAR_MAP:
      texture.type = "specular_map";
      specular_map = texture;
      break;
  }
}

inline bool compare_floats(float a, float b, float error=0.001) {
  return (glm::abs(a-b) <= error);
}

inline bool compare_vec3(glm::vec3 a, glm::vec3 b, float error=0.001) {
  return compare_floats(a.x, b.x, error) &&
         compare_floats(a.y, b.y, error) &&
         compare_floats(a.z, b.z, error);
}

bool Material::operator==(const Material& other_material) {
  // Check if values match
  if (compare_floats(metalness, other_material.metalness) &&
    compare_vec3(albedo, other_material.albedo) &&
    compare_vec3(ambient, other_material.ambient) &&
    compare_floats(specularity, other_material.specularity) &&
    compare_floats(shininess, other_material.shininess)) {
    // Check if there are the same # of albedo maps
    // Also check if specular maps and ambient maps match
    if (albedo_maps.size() == other_material.albedo_maps.size() &&
      specular_map.path == other_material.specular_map.path &&
      ambient_occlusion_map.path == other_material.ambient_occlusion_map.path) {
      // Check if all the albedo maps match
      for (unsigned int i=0; i<albedo_maps.size(); i++) {
        // Check if each albedo map matches (must be same order)
        if (albedo_maps[i].path != other_material.albedo_maps[i].path) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}
