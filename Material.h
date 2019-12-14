#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct Texture {
  unsigned int id;
  std::string path;
};

class Material : public QObject {
  Q_OBJECT

public:
  Material();
  ~Material();

  void set_materials(Shader *shader);

  // Defaults to 0 (non-metallic)
  float metalness;

  // Defaults will be empty (id=0,path="")
  vector<Texture> albedo_maps;
  Texture ambient_occlusion_map;
  Texture specular_map;

  // All values default to 1.0f
  glm::vec3 albedo;
  glm::vec3 ambient;
  float specularity;

  // Defaults to 64.0f
  float shininess;
};

#endif
