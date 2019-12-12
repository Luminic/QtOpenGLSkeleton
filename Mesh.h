#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Object.h"
#include "Shader.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coordinate;
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh : public Object {
  Q_OBJECT

public:
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures, Material material, glm::mat4 transformation=glm::mat4(1.0f));
  ~Mesh();

  void draw(Shader *shader, glm::mat4 model);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

private:
  void setup_mesh();
  glm::mat4 transformation;
};

#endif
