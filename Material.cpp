#include "Material.h"

Material::Material() :
  albedo(glm::vec3(1.0f)),
  ambient(glm::vec3(1.0f)),
  specularity(1.0f),
  shininess(64.0f)
{}

Material::~Material() {}

void Material::set_materials(Shader *shader) {
  shader->use();
  shader->setFloat("material.metalness", metalness);
  shader->setInt("material.number_albedo_maps", albedo_maps.size());
  shader->setBool("material.use_specular_map", specular_map.id!=0);
  shader->setBool("material.use_ambient_occlusion_map", false);

  shader->setVec3("material.albedo", albedo);
  shader->setVec3("material.ambient", ambient);
  shader->setFloat("material.specularity", specularity);

  shader->setFloat("material.shininess", shininess);
}
