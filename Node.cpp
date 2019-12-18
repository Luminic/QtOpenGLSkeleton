#include <QDebug>

#include "Node.h"

Node::Node(glm::vec3 position, glm::vec3 scale, glm::mat4 transformation) :
  position(position),
  scale(scale),
  transformation(transformation)
{}

Node::~Node() {
  for (auto m : meshes)
    delete m;

  for (auto c : child_nodes)
    delete c;
}

void Node::draw(Shader *shader, glm::mat4 model) {
  model *= transformation;
  shader->use();
  shader->setMat4("model", model);
  for (unsigned int i=0; i<meshes.size(); i++) {
    meshes[i]->draw(shader);
  }
  for (unsigned int i=0; i<child_nodes.size(); i++) {
    child_nodes[i]->draw(shader, model);
  }
}

// Getters
glm::mat4 Node::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::scale(model, glm::vec3(scale));
  return model;
}
