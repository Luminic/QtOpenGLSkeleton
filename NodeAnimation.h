#ifndef NODE_ANIMATION_H
#define NODE_ANIMATION_H

#include <QObject>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <unordered_map>

struct VectorKey {
  float animation_time;
  glm::vec3 vector;

  bool operator==(const VectorKey& other) {return vector==other.vector;}
  bool operator!=(const VectorKey& other) {return vector!=other.vector;}

  bool operator<(const VectorKey& other) {return animation_time<other.animation_time;}
  bool operator>(const VectorKey& other) {return animation_time>other.animation_time;}
};

struct QuaternionKey {
  float animation_time;
  glm::quat quaternion;

  bool operator==(const QuaternionKey& other) {return quaternion==other.quaternion;}
  bool operator!=(const QuaternionKey& other) {return quaternion!=other.quaternion;}

  bool operator<(const QuaternionKey& other) {return animation_time<other.animation_time;}
  bool operator>(const QuaternionKey& other) {return animation_time>other.animation_time;}
};

class NodeAnimationChannel : public QObject {
  Q_OBJECT;

protected:
  friend class Settings;
  // These should be in chronological order
  std::vector<VectorKey> position_keys;
  std::vector<QuaternionKey> rotation_keys;
  std::vector<VectorKey> scale_keys;

public:
  NodeAnimationChannel(std::string name);
  ~NodeAnimationChannel();

  std::string name;

  virtual void verify();

  virtual unsigned int find_position_index(float animation_time);
  virtual unsigned int find_rotation_index(float animation_time);
  virtual unsigned int find_scale_index(float animation_time);

  virtual glm::vec3 interpolate_position(float animation_time);
  virtual glm::quat interpolate_rotation(float animation_time);
  virtual glm::vec3 interpolate_scale(float animation_time);

  // TODO: Make these functions force the key to be inserted in proper chronological order
  virtual void add_position_key(VectorKey key) {position_keys.push_back(key);}
  virtual void add_rotation_key(QuaternionKey key) {rotation_keys.push_back(key);}
  virtual void add_scale_key(VectorKey key) {scale_keys.push_back(key);}
};

class NodeAnimation : public QObject {
  Q_OBJECT;

public:
  NodeAnimation(float tps, unsigned int duration, std::string name);
  ~NodeAnimation();

  NodeAnimationChannel* get_animation_channel_for(std::string node_name);

  std::string name;

  float tps;
  unsigned int duration;

  std::unordered_map<std::string, NodeAnimationChannel*> animation_channels;
};

#endif
