#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QDebug>
#include <vector>
#include <string>
#include <memory>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "Shader.h"

struct Transparent_Draw;

class Node : public QObject {
  Q_OBJECT

public:
  Node(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  ~Node();

  std::string name;

  virtual void draw(Shader* opaque_shader, Shader* full_transparency_shader=nullptr, Shader* partial_transparency_shader=nullptr, std::vector<Transparent_Draw>* partially_transparent_meshes=nullptr, glm::mat4 model=glm::mat4(1.0f), bool use_material=true, int texture_unit=0);

  // Getters & setters
  virtual glm::mat4 get_model_matrix();

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

  virtual void set_visibility(bool v);
  virtual bool get_visibility() {return visible;};

protected:
  friend class Settings;

  std::vector<std::shared_ptr<Mesh>> meshes;
  std::vector<std::shared_ptr<Node>> child_nodes;

  glm::mat4 transformation;
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation; // Yaw Pitch Roll represented by xyz

  bool visible;
};

#endif
