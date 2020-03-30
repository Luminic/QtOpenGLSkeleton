#ifndef ROOT_NODE_H
#define ROOT_NODE_H

#include "Node.h"

class RootNode : public Node {
  Q_OBJECT;

public:
  RootNode(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  ~RootNode();

  virtual void update_armature(int time, glm::mat4 parent_transformation=glm::mat4(1.0f)) override;
  virtual void draw(Shader_Opacity_Triplet shaders, std::vector<Transparent_Draw>* partially_transparent_meshes=nullptr, glm::mat4 model=glm::mat4(1.0f), bool use_material=true, int texture_unit=0) override;

  virtual const std::vector<Bone>& get_armature() {return armature;}
  virtual void set_bone_final_transform(unsigned int bone_index, const glm::mat4& final_transform);
  virtual const std::unordered_map<std::string, NodeAnimation*>& get_animation() {return animation;}

protected:
  // Armature exsts so the all bone matrices for this node tree can be sent to a shader
  std::vector<Bone> armature;
  std::unordered_map<std::string, NodeAnimation*> animation;

  glm::mat4 root_inverse_model;
};

#endif
