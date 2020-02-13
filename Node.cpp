#include <QDebug>

#include "Node.h"

Node::Node(glm::mat4 transformation, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) :
  transformation(transformation),
  position(position),
  scale(scale),
  rotation(rotation)
{}

Node::~Node() {
}

void Node::draw(Shader *shader, glm::mat4 model, bool use_material, int texture_unit) {
  model *= get_model_matrix();
  shader->use();
  shader->setMat4("model", model);
  for (unsigned int i=0; i<meshes.size(); i++) {
    meshes[i]->draw(shader, use_material, texture_unit);
  }
  for (unsigned int i=0; i<child_nodes.size(); i++) {
    child_nodes[i]->draw(shader, model, use_material, texture_unit);
  }
}

// Getters & setters
glm::mat4 Node::get_model_matrix() {
  glm::mat4 model = glm::translate(transformation, position);
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(0.0f,1.0f,0.0f));
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(1.0f,0.0f,0.0f));
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f,0.0f,1.0f));
  model = glm::scale(model, glm::vec3(scale));
  return model;
}

void Node::add_mesh(std::shared_ptr<Mesh> mesh) {
  meshes.push_back(mesh);
}
void Node::delete_mesh_at(unsigned int index) {
  Q_ASSERT_X(index < meshes.size(), "delete_mesh_at", "index is greater than vector meshes' size");
  meshes.erase(meshes.begin() + index);
}
void Node::clear_meshes() {
  meshes.clear();
}

void Node::add_child_node(std::shared_ptr<Node> node) {
  child_nodes.push_back(node);
}
void Node::delete_child_node_at(unsigned int index) {
  Q_ASSERT_X(index < child_nodes.size(), "delete_child_node_at", "index is greater than vector child_nodes' size");
  child_nodes.erase(child_nodes.begin() + index);
}
void Node::clear_child_nodes() {
  child_nodes.clear();
}

void Node::set_transformation(glm::mat4 transf) {
  transformation = transf;
}
const glm::mat4& Node::get_transformation() {
  return transformation;
}
void Node::set_position(glm::vec3 pos) {
  position = pos;
}
const glm::vec3& Node::get_position() {
  return position;
}
void Node::set_scale(glm::vec3 sca) {
  scale = sca;
}
const glm::vec3& Node::get_scale() {
  return scale;
}
void Node::set_rotation(glm::vec3 rot) {
  rotation = rot;
}
const glm::vec3& Node::get_rotation() {
  return rotation;
}
