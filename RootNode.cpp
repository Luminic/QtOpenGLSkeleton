#include "RootNode.h"

RootNode::RootNode(glm::mat4 transformation, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) : Node(transformation, position, scale, rotation) {
  root_node = this;
}

RootNode::~RootNode() {
  for (auto it : animation) {
    delete it.second;
  }
}

void RootNode::update_armature(int time, glm::mat4 parent_transformation) {
  root_inverse_model = inverse(get_model_matrix());
  if (armature.size() >= 1) {
    Node::update_armature(time, parent_transformation);
  }
}

void RootNode::draw(Shader_Opacity_Triplet shaders, std::vector<Transparent_Draw>* partially_transparent_meshes, glm::mat4 model, bool use_material, int texture_unit) {
  for (unsigned int i=0; i<armature.size(); i++) {
    shaders.setMat4(("armature["+std::to_string(i)+"]").c_str(), armature[i].final_transform);
  }

  Node::draw(shaders, partially_transparent_meshes, model, use_material, texture_unit);
}

void RootNode::set_bone_final_transform(unsigned int bone_index, const glm::mat4& parent_transformation) {
  armature[bone_index].final_transform = parent_transformation * armature[bone_index].offset * root_inverse_model;
}
