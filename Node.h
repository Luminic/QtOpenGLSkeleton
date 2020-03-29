#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QDebug>

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "NodeAnimation.h"

struct Transparent_Draw;

struct Bone {
  glm::mat4 offset;
  glm::mat4 final_transform;
};

class Node : public QObject {
  Q_OBJECT

public:
  Node(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  ~Node();

  std::string name;
  static int nr_nodes_created;

  virtual void update_armature(int time, Node* root_node=nullptr, glm::mat4 parent_transformation=glm::mat4(1.0f));
  virtual void draw(Shader_Opacity_Triplet shaders, std::vector<Transparent_Draw>* partially_transparent_meshes=nullptr, glm::mat4 model=glm::mat4(1.0f), bool use_material=true, int texture_unit=0);

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

  void set_animated(bool animated) {this->animated = animated;};
  bool get_animated() {return animated;};

  virtual const glm::mat4& get_root_inverse_model() {return root_inverse_model;}

protected:
  friend class Settings;

  std::vector<std::shared_ptr<Mesh>> meshes;
  std::vector<std::shared_ptr<Node>> child_nodes;

  // Only the root node should have this filled
  // armature exsts so the all bone matrices can be sent to a shader
  std::vector<Bone> armature;
  // If the node is a bone node, this will be the index of the bone in armature (the root node's armature. This node's armature should be empty)
  // If the node is a plain node, this will be -1
  int bone_id = -1;

  // Only the root node should have this filled
  std::unordered_map<std::string, NodeAnimation*> animations;

  bool animated = false;

  glm::mat4 transformation;
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation; // Yaw Pitch Roll represented by xyz

  bool visible;

  // The inverse of the final model matrix is stored here for the root node
  // It is undefined for non root nodes
  // If the inverse of a non-root node is needed, use inverse(get_model_matrix()) instead; this variable exists as a time saver
  glm::mat4 root_inverse_model;
};

#endif
