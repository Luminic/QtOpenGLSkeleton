#include "NodeAnimation.h"

#include <QDebug>

NodeAnimationChannel::NodeAnimationChannel(std::string name) {
  this->name = name;
}

NodeAnimationChannel::~NodeAnimationChannel() {}

void NodeAnimationChannel::verify() {
  #ifdef QT_DEBUG
    for (unsigned int i=1; i<position_keys.size(); i++) {
      Q_ASSERT_X(position_keys[i-1].animation_time < position_keys[i].animation_time, "Node animation verification", "position keys are out of order");
    }
    for (unsigned int i=1; i<rotation_keys.size(); i++) {
      Q_ASSERT_X(rotation_keys[i-1].animation_time < rotation_keys[i].animation_time, "Node animation verification", "rotation keys are out of order");
    }
    for (unsigned int i=1; i<scale_keys.size(); i++) {
      Q_ASSERT_X(scale_keys[i-1].animation_time < scale_keys[i].animation_time, "Node animation verification", "scale keys are out of order");
    }
  #endif
}

// TODO: make these use binary search instead of a linear search
unsigned int NodeAnimationChannel::find_position_index(float animation_time) {
  for (unsigned int i=0; i<position_keys.size()-1; i++) {
    if (animation_time < position_keys[i+1].animation_time) {
      return i;
    }
  }
  qFatal("Finding position at animation_time: Could not find position");
}

unsigned int NodeAnimationChannel::find_rotation_index(float animation_time) {
  for (unsigned int i=0; i<rotation_keys.size()-1; i++) {
    if (animation_time < rotation_keys[i+1].animation_time) {
      return i;
    }
  }
  qFatal("Finding position at animation_time: Could not find rotation");
}

unsigned int NodeAnimationChannel::find_scale_index(float animation_time) {
  for (unsigned int i=0; i<scale_keys.size()-1; i++) {
    if (animation_time < scale_keys[i+1].animation_time) {
      return i;
    }
  }
  qFatal("Finding position at animation_time: Could not find scale");
}


glm::vec3 NodeAnimationChannel::interpolate_position(float animation_time) {
  Q_ASSERT_X(position_keys.size() > 0, "Position interpolation", "No position keys");
  // qDebug() << animation_time;
  if (position_keys.size() == 1) {
    return position_keys[0].vector;
  }
  unsigned int index = find_position_index(animation_time);
  float delta_time = position_keys[index+1].animation_time - position_keys[index].animation_time;
  float factor = (animation_time - position_keys[index].animation_time) / delta_time;
  Q_ASSERT_X(0.0f <= factor && factor <= 1.0f, "Position interpolation", "Interpolation factor is out of bounds");

  return glm::mix(position_keys[index].vector, position_keys[index+1].vector, factor);
}

glm::quat NodeAnimationChannel::interpolate_rotation(float animation_time) {
  Q_ASSERT_X(rotation_keys.size() > 0, "Rotation interpolation", "No rotation keys");
  if (rotation_keys.size() == 1) {
    return rotation_keys[0].quaternion;
  }
  unsigned int index = find_rotation_index(animation_time);
  float delta_time = rotation_keys[index+1].animation_time - rotation_keys[index].animation_time;
  float factor = (animation_time - rotation_keys[index].animation_time)/delta_time;
  Q_ASSERT_X(0.0f <= factor && factor <= 1.0f, "Rotation interpolation", "Interpolation factor is out of bounds");
  return glm::normalize(glm::slerp(
    rotation_keys[index].quaternion,
    rotation_keys[index+1].quaternion,
    factor
  ));
}

glm::vec3 NodeAnimationChannel::interpolate_scale(float animation_time) {
  Q_ASSERT_X(scale_keys.size() > 0, "Scale interpolation", "No scale keys");
  if (scale_keys.size() == 1) {
    return scale_keys[0].vector;
  }
  unsigned int index = find_scale_index(animation_time);
  float delta_time = scale_keys[index+1].animation_time - scale_keys[index].animation_time;
  float factor = (animation_time - scale_keys[index].animation_time) / delta_time;
  Q_ASSERT_X(0.0f <= factor && factor <= 1.0f, "Scale interpolation", "Interpolation factor is out of bounds");

  return glm::mix(scale_keys[index].vector, scale_keys[index+1].vector, factor);
}

NodeAnimation::NodeAnimation(float tps, unsigned int duration, std::string name) {
  this->tps = tps;
  this->duration = duration;
  this->name = name;
}

NodeAnimation::~NodeAnimation() {
  for (auto it : animation_channels) {
    delete it.second;
  }
}

NodeAnimationChannel* NodeAnimation::get_animation_channel_for(std::string node_name) {
  auto it = animation_channels.find(node_name);
  Q_ASSERT_X(it != animation_channels.end(), "Getting animation channel", "Could not find requested animation channel");
  return it->second;
}
