#include <QDebug>

#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  color(color),
  ambient(ambient),
  diffuse(diffuse),
  specular(specular)
{
  this->position = position;
  this->scale = scale;

  constant = 1.0f;
  linear = 0.09f;
  quadratic = 0.032f;

  initialize_cube();
}

Light::~Light() {
}

void Light::set_object_settings(std::string name, Shader *shader) {
  shader->use();

  shader->setVec3((name+".position").c_str(), position);

  shader->setVec3((name+".ambient").c_str(), color*ambient);
  shader->setVec3((name+".diffuse").c_str(), color*diffuse);
  shader->setVec3((name+".specular").c_str(), color*specular);

  shader->setFloat((name+".constant").c_str(), constant);
  shader->setFloat((name+".linear").c_str(), linear);
  shader->setFloat((name+".quadratic").c_str(), quadratic);
}

void Light::draw(Shader *shader) {
  shader->use();
  shader->setVec3("color", color);
  shader->setMat4("model", get_model_matrix());
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
}

// Getters
glm::mat4 Light::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::scale(model, glm::vec3(scale));
  return model;
}
