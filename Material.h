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
  UNKNOWN,
  ALBEDO_MAP, // Should be in gamma space (will be converted into linear space)
  AMBIENT_OCCLUSION_MAP, // Should be in linear space
  SPECULAR_MAP, // Should be in linear space
  CUBE_MAP // Should be in gamma space (will not be converted into linear space)
};

struct Texture {
  unsigned int id;
  Image_Type type;
  std::string path;
};

class Material : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Material();
  ~Material();

  void set_materials(Shader *shader, int material_index_offset=0);

  unsigned int load_texture(const char *path, Image_Type type, bool add_to_material=true);
  unsigned int load_cubemap(std::vector<std::string> faces);

  bool operator==(const Material& other_material);

  // Defaults to 0 (non-metallic)
  float metalness;

  std::vector<Texture> textures;

  // All values default to 1.0f
  glm::vec3 albedo;
  glm::vec3 ambient;
  float specularity;

  // Defaults to 64.0f
  float shininess;
};

#endif
