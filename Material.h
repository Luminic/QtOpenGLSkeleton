#ifndef MATERIAL_H
#define MATERIAL_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

enum Transparency {
  OPAQUE = 0,                 // Everything is opaque
  FULL_TRANSPARENCY = 1,      // Some parts are fully transparent (fragment discarding must be enabled)
  PARTIAL_TRANSPARENCY = 2    // Some parts are partially transparent (fragment blending must be enabled)
};

namespace ImageLoading {
  enum Options : char {
    NONE            = 0,
    TRANSPARENCY    = 1 << 0,
    FLIP_ON_LOAD    = 1 << 1,
    CLAMPED         = 1 << 2,
  };
  inline constexpr Options operator|(Options a, Options b) {
    return a = static_cast<Options> (int(a) | int(b));
  }
  inline constexpr Options operator&(Options a, Options b) {
    return a = static_cast<Options> (int(a) & int(b));
  }
}

struct Texture {
  unsigned int id;
  Image_Type type;
  std::string path;
  QImage image;
};

class Material : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT;

public:
  std::string name;
  static int nr_materials_created;

  std::vector<Texture> textures;
  Texture opacity_map;

  // Defaults are in comments
  glm::vec3 color;
  float ambient; // 0.2f
  float diffuse; // 1.0f
  float specular; // 1.0f
  float roughness; // 1.0f
  float metalness; // 0.0

  Material(std::string name);
  Material();
  ~Material();

  int draw(Shader* shader, int texture_unit);
  void set_opacity_map(Shader* shader, int& texture_unit); // Assumes shader is already in use

  Texture load_texture(const char *path, Image_Type type, ImageLoading::Options options=ImageLoading::Options::NONE);
  static Texture static_load_texture(const char *path, Image_Type type, ImageLoading::Options options=ImageLoading::Options::NONE);
  Texture load_cubemap(const std::vector<std::string>& faces, bool add_to_material=true);

  bool operator==(const Material& other_material);

protected:
  // Helper functions
  void init();
  void set_textures(Shader* shader, int& texture_unit);
};

#endif
