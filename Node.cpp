#include <QDebug>

#include "Node.h"

Node::Node(glm::mat4 transformation, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) :
  transformation(transformation),
  position(position),
  scale(scale),
  rotation(rotation)
{
  visible = true;
}

Node::~Node() {
}

void Node::draw(Shader* opaque_shader, Shader* full_transparency_shader, Shader* partial_transparency_shader, glm::mat4 model, bool use_material, int texture_unit) {
  if (visible) {
    model *= get_model_matrix();
    opaque_shader->use();
    opaque_shader->setMat4("model", model);
    if (full_transparency_shader != nullptr) {
      full_transparency_shader->use();
      full_transparency_shader->setMat4("model", model);
    }
    for (unsigned int i=0; i<meshes.size(); i++) {
      if (meshes[i]->get_transparency() == OPAQUE)
        meshes[i]->draw(opaque_shader, use_material, texture_unit);
      else if (meshes[i]->get_transparency() == FULL_TRANSPARENCY) {
        Q_ASSERT_X(full_transparency_shader != nullptr, "Node::draw", "full_transparency_shader is null but it is needed");
        meshes[i]->draw(full_transparency_shader, use_material, texture_unit);
      } else {

      }
    }
    for (unsigned int i=0; i<child_nodes.size(); i++) {
      child_nodes[i]->draw(opaque_shader, full_transparency_shader, partial_transparency_shader, model, use_material, texture_unit);
    }
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

void Node::set_visibility(bool v) {
  visible = v;
}
