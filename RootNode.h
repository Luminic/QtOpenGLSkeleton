#ifndef ROOT_NODE_H
#define ROOT_NODE_H

#include <QElapsedTimer>

#include "Node.h"

class RootNode : public Node {
  Q_OBJECT;

public:
  enum Animation_Status {
    NO_ANIMATION = 0x00, // Translate using bone matrices
    ANIMATION_PAUSED = 0x01, // Translate using animation data (animation is paused)
    ANIMATED = 0x02 // Translate using animation data (animation is on)
  };

protected:
  friend class Settings;
  // Armature exsts so the all bone matrices for this node tree can be sent to a shader
  std::vector<Bone> armature;
  std::unordered_map<std::string, NodeAnimation*> animation;

  glm::mat4 root_inverse_model;

  Animation_Status animation_status = NO_ANIMATION;
  NodeAnimation* current_animation = nullptr;
  std::string current_animation_name = "";
  QElapsedTimer* timer = new QElapsedTimer();
  int time_offset;

public:
  RootNode(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  ~RootNode();

  virtual void update();
  virtual void draw(Shader_Opacity_Triplet shaders, std::vector<Transparent_Draw>* partially_transparent_meshes=nullptr, glm::mat4 model=glm::mat4(1.0f), bool use_material=true, int texture_unit=0) override;

  virtual const std::vector<Bone>& get_armature() {return armature;}
  virtual void set_bone_final_transform(unsigned int bone_index, const glm::mat4& final_transform);
  virtual const std::unordered_map<std::string, NodeAnimation*>& get_animation() {return animation;}

  virtual void set_current_animation(std::string new_animation_name);
  virtual std::string get_current_animation_name() {return current_animation_name;}
  virtual NodeAnimation* get_current_animation() {return current_animation;}

  // Note that time and animation time are different
  virtual void disable_animation();
  virtual void start_animation(); // Starts/resumes the animation
  virtual void stop_animation(); // Stops the animation. Offset is set to the current time.
  virtual Animation_Status get_animation_status() {return animation_status;}
  virtual float get_animation_time(); // Will complain if current_animation is not set
  virtual void set_animation_time(float animation_time); // Will complain if current_animation is not set; Animation time is set to 0 when current_animation is set

signals:
  void animation_status_changed(Animation_Status new_status);
  void animation_changed(NodeAnimation* new_animation);

};

#endif
