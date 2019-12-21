#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"

class Node : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Node(glm::mat4 transformation=glm::mat4(1.0f), glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 rotation=glm::vec3(0.0f));
  ~Node();

  void draw(Shader *shader, glm::mat4 model, int material_index_offset=0);

  // Getters
  glm::mat4 get_model_matrix();

  std::vector<Mesh*> meshes;
  std::vector<Node*> child_nodes;

  glm::mat4 transformation;
  glm::vec3 position;
  glm::vec3 scale;
  glm::vec3 rotation; // Yaw Pitch Roll represented by xyz

  std::string name;
};

#endif
