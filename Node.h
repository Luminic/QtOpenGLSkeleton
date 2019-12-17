#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"

class Node : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Node(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::mat4 transformation=glm::mat4(1.0f));
  ~Node();

  void draw(Shader *shader, glm::mat4 model);

  // Getters
  glm::mat4 get_model_matrix();
  glm::vec3 get_position();
  glm::vec3 get_scale();

  std::vector<Mesh*> meshes;
  std::vector<Node*> child_nodes;

  glm::mat4 transformation;
  glm::vec3 position;
  glm::vec3 scale;

public slots:
  // Setters
  void set_position(glm::vec3 position);
  void set_x_position(float x);
  void set_y_position(float y);
  void set_z_position(float z);
  void set_scale(glm::vec3 scale);
  void set_x_scale(float x);
  void set_y_scale(float y);
  void set_z_scale(float z);
};

#endif
