#include <QDebug>

#include <glm/gtx/quaternion.hpp>

#include <cmath>

#include "Node.h"
#include "Scene.h"

int Node::nr_nodes_created = 0;

Node::Node(glm::mat4 transformation, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) :
  transformation(transformation),
  position(position),
  scale(scale),
  rotation(rotation)
{
  name = "node #" + std::to_string(nr_nodes_created);
  visible = true;
}

Node::~Node() {}

void Node::update_armature(int time, glm::mat4 parent_transformation) {
  Q_ASSERT_X(root_node != nullptr, "Updating armature", "Root node is not set");

  if (!animated) {
    parent_transformation *= get_model_matrix();
  } else {
    auto it = root_node->get_animation().find(std::string("Armature|ArmatureAction"));
    Q_ASSERT_X(it != root_node->get_animation().end(), "Finding animation", "Could not find requested animation");
    NodeAnimation* node_animation = it->second;
    NodeAnimationChannel* node_animation_channel = node_animation->get_animation_channel_for(this->name);

    float animation_time = time / 1000.0f * node_animation->tps;
    animation_time = fmod(animation_time, node_animation->duration);
    parent_transformation = glm::translate(parent_transformation, node_animation_channel->interpolate_position(animation_time));
    parent_transformation *= glm::toMat4(node_animation_channel->interpolate_rotation(animation_time));
    parent_transformation = glm::scale(parent_transformation, node_animation_channel->interpolate_scale(animation_time));
    parent_transformation *= get_model_matrix(false);
  }

  if (bone_id >= 0) {
    root_node->set_bone_final_transform(bone_id, parent_transformation);
  }

  for (auto node : child_nodes) {
    node->update_armature(time, parent_transformation);
  }
}

void Node::draw(Shader_Opacity_Triplet shaders, std::vector<Transparent_Draw>* partially_transparent_meshes, glm::mat4 model, bool use_material, int texture_unit) {
  if (visible) {
    model *= get_model_matrix();
    shaders.opaque->use();
    shaders.opaque->setMat4("model", model);
    if (shaders.full_transparency != nullptr) {
      shaders.full_transparency->use();
      shaders.full_transparency->setMat4("model", model);
    }
    // Partially transparent objects should only be drawn immediately for shadow maps
    // For normal rendering, they should be rendered later
    if ((!use_material) && shaders.partial_transparency != nullptr) {
      shaders.partial_transparency->use();
      shaders.partial_transparency->setMat4("model", model);
    }

    for (unsigned int i=0; i<meshes.size(); i++) {
      if (meshes[i]->get_transparency() == OPAQUE)
        meshes[i]->draw(shaders.opaque, use_material, texture_unit);
      else if (meshes[i]->get_transparency() == FULL_TRANSPARENCY) {
        Q_ASSERT_X(shaders.full_transparency != nullptr, "Node::draw", "shaders.full_transparency is null but it is needed");
        meshes[i]->draw(shaders.full_transparency, use_material, texture_unit);
      } else {
        Q_ASSERT_X(shaders.partial_transparency != nullptr, "Node::draw", "shaders.partial_transparency is null but it is needed");
        Q_ASSERT_X(partially_transparent_meshes != nullptr, "Node::draw", "partially_transparent_meshes is null but it is needed");
        if (!use_material) {
          meshes[i]->draw(shaders.partial_transparency, use_material, texture_unit);
        } else {
          partially_transparent_meshes->push_back(Transparent_Draw{meshes[i].get(), model, texture_unit});
        }
      }
    }
    for (unsigned int i=0; i<child_nodes.size(); i++) {
      child_nodes[i]->draw(shaders, partially_transparent_meshes, model, use_material, texture_unit);
    }
  }
}

// Getters & setters
glm::mat4 Node::get_model_matrix(bool use_transformation_matrix) {
  glm::mat4 model = use_transformation_matrix ? transformation : glm::mat4(1.0f);
  model = glm::translate(model, position);
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
