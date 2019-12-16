#include <QDebug>

#include "Object.h"

Object::Object(glm::vec3 position, glm::vec3 scale) :
  position(position),
  scale(scale)
{}

Object::~Object() {
}

// Getters
glm::mat4 Object::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::scale(model, glm::vec3(scale));
  return model;
}

glm::vec3 Object::get_position() {return position;}
glm::vec3 Object::get_scale() {return scale;}
unsigned int Object::get_VBO() {return VBO;}
unsigned int Object::get_EBO() {return EBO;}

// Setters
void Object::set_position(glm::vec3 position) {this->position=position;}
void Object::set_x_position(float x) {position.x=x;}
void Object::set_y_position(float y) {position.y=y;}
void Object::set_z_position(float z) {position.z=z;}
void Object::set_scale(glm::vec3 scale) {this->scale=scale;}
void Object::set_x_scale(float x) {scale.x=x;}
void Object::set_y_scale(float y) {scale.y=y;}
void Object::set_z_scale(float z) {scale.z=z;}
