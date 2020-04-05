#include <QDebug>

#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  Node(glm::mat4(1.0f), position, scale, glm::vec3(1.0f)),
  color(color),
  ambient(ambient),
  diffuse(diffuse),
  specular(specular)
{
  initializeOpenGLFunctions();
}

Light::~Light() {
}

void Light::set_object_settings(std::string name, Shader *shader) {
  shader->setVec3((name+".color").c_str(), color);
  shader->setFloat((name+".ambient").c_str(), ambient);
  shader->setFloat((name+".diffuse").c_str(), diffuse);
  shader->setFloat((name+".specular").c_str(), specular);
}

void Light::draw(Shader *shader, glm::mat4 model, bool use_material, int texture_unit) {
  if (visible) {
    shader->use();
    shader->setVec3("color", color);
    shader->setMat4("model", model*get_model_matrix());
    for (auto mesh : meshes) {
      mesh->simple_draw();
    }
  }
}
