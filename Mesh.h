#ifndef MESH_H
#define MESH_H

#include <QObject>
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

enum Transparency {
  OPAQUE = 0,                 // Everything is opaque
  FULL_TRANSPARENCY = 1,      // Some parts are fully transparent (fragment discarding must be enabled)
  PARTIAL_TRANSPARENCY = 2    // Some parts are partially transparent (fragment blending must be enabled)
};

class Mesh : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material);
  Mesh();
  ~Mesh();

  std::string name;

  void initialize_cube(float texture_scale=1.0f);
  void initialize_plane(bool horizontal=true, float texture_scale=1.0f);
  void initialize_buffers();

  void draw(Shader *shader, bool use_material=true, int texture_unit=0);

  Transparency get_transparency() {return transparency;};
  void set_transparency(Transparency new_transparency) {transparency=new_transparency;};

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Material *material;

protected:
  glm::mat4 transformation;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;

  Transparency transparency;
};

#endif
