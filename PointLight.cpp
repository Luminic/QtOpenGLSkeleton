#include <QDebug>

#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  color(color),
  ambient(ambient),
  diffuse(diffuse),
  specular(specular)
{
  this->position = position;
  this->scale = scale;
  initialize_cube();
}

PointLight::~PointLight() {
}

void PointLight::set_object_settings(std::string name, Shader *shader) {
  shader->use();

  shader->setVec3((name+".position").c_str(), position);

  shader->setVec3((name+".ambient").c_str(), color*ambient);
  shader->setVec3((name+".diffuse").c_str(), color*diffuse);
  shader->setVec3((name+".specular").c_str(), color*specular);

  shader->setFloat((name+".constant").c_str(), constant);
  shader->setFloat((name+".linear").c_str(), linear);
  shader->setFloat((name+".quadratic").c_str(), quadratic);
}

void PointLight::draw(Shader *shader) {
  shader->use();
  shader->setVec3("color", color);
  shader->setMat4("model", get_model_matrix());
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
}

// Getters
glm::vec3 PointLight::get_color() {return color;}
float PointLight::get_ambient() {return ambient;}
float PointLight::get_diffuse() {return diffuse;}
float PointLight::get_specular() {return specular;}
float PointLight::get_constant() {return constant;}
float PointLight::get_linear() {return linear;}
float PointLight::get_quadratic() {return quadratic;}

//Setters
void PointLight::set_falloff(float constant, float linear, float quadratic) {
  this->constant = constant;
  this->linear = linear;
  this->quadratic = quadratic;
}

void PointLight::set_color(glm::vec3 color) {this->color = color;}
void PointLight::set_red(float red) {color.r = red;}
void PointLight::set_green(float green) {color.g = green;}
void PointLight::set_blue(float blue) {color.b = blue;}
void PointLight::set_ambient(float ambient) {this->ambient = ambient;}
void PointLight::set_diffuse(float diffuse) {this->diffuse = diffuse;}
void PointLight::set_specular(float specular) {this->specular = specular;}
void PointLight::set_constant(float constant) {this->constant = constant;}
void PointLight::set_linear(float linear) {this->linear = linear;}
void PointLight::set_quadratic(float quadratic) {this->quadratic = quadratic;}
