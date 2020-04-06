#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QDebug>

#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "NodeAnimation.h"
#include "Shader.h"

struct Transparent_Draw;

struct Bone {
  glm::mat4 offset;
  glm::mat4 final_transform;
};

class RootNode;

class Node : public QObject {
  Q_OBJECT;

protected:
  friend class Settings;

  std::set<Shader_Opacity_Triplet> relevant_color_shaders;
  std::set<DepthShaderGroup> relevant_depth_shaders;

  std::vector<std::shared_ptr<Mesh>> meshes;
  std::vector<std::shared_ptr<Node>> child_nodes;

  // If the node is a bone node, this will be the index of the bone in armature (the root node's armature)
  // If the node is a plain node, this will be -1
  int bone_id = -1;

  bool has_animation = false;

  glm::mat4 transformation;
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation; // Yaw Pitch Roll represented by xyz

  bool visible;

public:
  std::string name;
  static int nr_nodes_created;

  Node(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  virtual ~Node();

  // Traverse the node tree to update and return this node's relevant shaders (will update the relevant shaders in all children nodes)
  virtual const std::set<Shader_Opacity_Triplet>& update_relevant_color_shaders();
  virtual const std::set<DepthShaderGroup>& update_relevant_depth_shaders();

  // If NodeAnimation is a nullptr, bone matrix data is used instead of animation data (i.e. default bone pose is used)
  virtual void update_armature(glm::mat4 parent_transformation, RootNode* root_node, NodeAnimation* animation, int animation_time);
  virtual void draw(Shader::DrawType draw_type, std::vector<Transparent_Draw>* partially_transparent_meshes=nullptr, glm::mat4 model=glm::mat4(1.0f), int texture_unit=0);

  // Getters & setters
  virtual glm::mat4 get_model_matrix(bool use_transformation_matrix=true);

  virtual const std::vector<std::shared_ptr<Mesh>>& get_meshes() const {return meshes;}
  virtual void add_mesh(std::shared_ptr<Mesh> mesh);
  virtual void delete_mesh_at(unsigned int index);
  virtual void clear_meshes();

  virtual const std::vector<std::shared_ptr<Node>>& get_child_nodes() const {return child_nodes;}
  virtual void add_child_node(std::shared_ptr<Node> node);
  virtual void delete_child_node_at(unsigned int index);
  virtual void clear_child_nodes();

  virtual void set_transformation(glm::mat4 transf);
  virtual const glm::mat4& get_transformation();
  virtual void set_position(glm::vec3 pos);
  virtual const glm::vec3& get_position();
  virtual void set_scale(glm::vec3 sca);
  virtual const glm::vec3& get_scale();
  virtual void set_rotation(glm::vec3 rot);
  virtual const glm::vec3& get_rotation();

  virtual void set_bone_id(int id) {bone_id = id;}

  virtual void set_visibility(bool v);
  virtual bool get_visibility() {return visible;}

  void set_animated(bool has_animation) {this->has_animation = has_animation;};
  bool get_animated() {return has_animation;};
};

#endif
