#include <cmath>

#include "RootNode.h"

RootNode::RootNode(glm::mat4 transformation, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) : Node(transformation, position, scale, rotation) {
  root_node = this;
}

RootNode::~RootNode() {
  for (auto it : animation) {
    delete it.second;
  }
}

void RootNode::update() {
  root_inverse_model = inverse(get_model_matrix());
  if (armature.size() >= 1) {
    update_armature(glm::mat4(1.0f), animation_status==Animation_Status::NO_ANIMATION ? nullptr : current_animation, get_animation_time());
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

void RootNode::set_current_animation(std::string new_animation_name) {
  if (new_animation_name.empty()) {
    if (animation_status==NO_ANIMATION) {
      current_animation_name.clear();
      current_animation = nullptr;
      return;
    }
    Q_ASSERT_X(0, "Setting current animation", "Animation status must be set to NO_ANIMATION before animation can be cleaned");
  }
  auto it = animation.find(new_animation_name);
  Q_ASSERT_X(it != animation.end(), "Setting current animation", "Could not find specified animation in animations map");
  current_animation_name=new_animation_name;
  current_animation = it->second;
  emit animation_changed(current_animation);
  time_offset=0;
}

void RootNode::disable_animation() {
  animation_status = Animation_Status::NO_ANIMATION;
  emit animation_status_changed(animation_status);
}

void RootNode::start_animation() {
  Q_ASSERT_X(current_animation!=nullptr, "Start animation", "No current animation");
  timer->start();
  animation_status = Animation_Status::ANIMATED;
  emit animation_status_changed(animation_status);
}

void RootNode::stop_animation() {
  if (animation_status == Animation_Status::ANIMATED) {
    time_offset += timer->elapsed();
    animation_status = Animation_Status::ANIMATION_PAUSED;
    emit animation_status_changed(animation_status);
  }
}

float RootNode::get_animation_time() {
  if (animation_status == Animation_Status::NO_ANIMATION) {
    return 0.0f;
  }
  int time = time_offset;
  if (animation_status == Animation_Status::ANIMATED) {
    time += timer->elapsed();
  }
  float animation_time = time / 1000.0f * current_animation->tps;
  animation_time = fmod(animation_time, current_animation->duration);
  return animation_time;
}

void RootNode::set_animation_time(float animation_time) {
  time_offset = animation_time * 1000.0f / current_animation->tps;
}
