#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

enum Image_Type {
  ALBEDO_MAP, // Should be in gamma space (will be converted into linear space)
  AMBIENT_OCCLUSION_MAP, // Should be in linear space
  SPECULAR_MAP // Should be in linear space
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};

class Material : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Material();
  ~Material();

  void set_materials(Shader *shader);

  void load_texture(const char *path, Image_Type type);

  bool operator==(const Material& other_material);

  // Defaults to 0 (non-metallic)
  float metalness;

  // Defaults will be empty (id=0,path="")
  std::vector<Texture> albedo_maps;
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