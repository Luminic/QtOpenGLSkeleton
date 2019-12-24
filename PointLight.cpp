#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic) :
  Light(position, scale, color, ambient, diffuse, specular),
  constant(constant),
  linear(linear),
  quadratic(quadratic)
{}

PointLight::~PointLight() {
}

void PointLight::set_object_settings(std::string name, Shader *shader) {
  Light::set_object_settings(name, shader);

  shader->setVec3((name+".position").c_str(), position);

  shader->setFloat((name+".constant").c_str(), constant);
  shader->setFloat((name+".linear").c_str(), linear);
  shader->setFloat((name+".quadratic").c_str(), quadratic);
}
