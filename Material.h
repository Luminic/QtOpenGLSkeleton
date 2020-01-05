#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

enum Image_Type {
  UNKNOWN,
  ALBEDO_MAP, // Should be in gamma space (will be converted into linear space)
  AMBIENT_OCCLUSION_MAP, // Should be in linear space
  ROUGHNESS_MAP, // Should be in linear space
  METALNESS_MAP, // Should be in linear space
  CUBE_MAP // Should be in gamma space (will be converted into linear space)
};

struct Texture {
  unsigned int id;
  Image_Type type;
  std::string path;
};

class Material : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  Material();
  ~Material();

  void set_materials(Shader *shader, int material_index_offset=0);

  Texture load_texture(const char *path, Image_Type type, bool add_to_material=true);
  Texture load_cubemap(std::vector<std::string> faces, bool add_to_material=true);

  bool operator==(const Material& other_material);

  std::vector<Texture> textures;

  // Defaults are in comments
  glm::vec3 color;
  float ambient; // 0.2f
  float diffuse; // 1.0f
  float specular; // 1.0f
  float roughness; // 1.0f
  float metalness; // 0.0

  // For use with settings class
  int index;
};

#endif
