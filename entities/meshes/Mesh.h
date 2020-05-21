#ifndef MESH_H
#define MESH_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Material.h"
#include "../../rendering/Shader.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coordinate;
  glm::ivec4 bone_ids;
  glm::vec4 bone_weights;
};

class Mesh : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material);
  Mesh();
  ~Mesh();
  void init();

  std::string name;
  static int nr_meshes_created;

  void initialize_cube(float texture_scale=1.0f);
  void initialize_plane(bool horizontal=true, float texture_scale=1.0f);
  void initialize_buffers();

  void draw(Shader* shader, Shader::DrawType draw_type, const glm::mat4& model, int texture_unit=0);
  void simple_draw(); // Just draws the object to the screen. The shader should be set before calling this.

  Transparency get_transparency() {return transparency;};
  void set_transparency(Transparency new_transparency) {transparency=new_transparency;};

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Material* material = nullptr;

protected:
  glm::mat4 transformation;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;

  Transparency transparency;
};

#endif
