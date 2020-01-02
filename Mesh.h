#ifndef MESH_H
#define MESH_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Material.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coordinate;
};

class Mesh : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material);
  Mesh();
  ~Mesh();

  void initialize_cube(float texture_scale=1.0f);
  void initialize_plane(bool horizontal=true, float texture_scale=1.0f);
  void initialize_buffers();

  void draw(Shader *shader, bool use_material=true, int material_index_offset=0);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Material *material;

  std::string name;

protected:
  glm::mat4 transformation;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
};

#endif
