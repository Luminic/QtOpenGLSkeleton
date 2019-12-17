#ifndef MESH_H
#define MESH_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Material.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coordinate;
};

class Mesh : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material, glm::mat4 transformation=glm::mat4(1.0f));
  Mesh();
  ~Mesh();

  void initialize_cube();
  void initialize_buffers();

  void draw(Shader *shader, glm::mat4 model);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Material *material;

protected:
  glm::mat4 transformation;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
};

#endif
