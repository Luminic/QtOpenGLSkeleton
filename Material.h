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
  CUBE_MAP, // Should be in gamma space (will be converted into linear space)
  OPACITY_MAP // Only used when a mesh w/ transparent arts is drawn. Only the alpha value is used
};

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
    ADD_TO_MATERIAL = 1 << 3
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

protected:
  Shader_Opacity_Triplet color_shaders;
  DepthShaderGroup depth_shaders;

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

  Material(std::string name, Shader_Opacity_Triplet color_shaders, DepthShaderGroup depth_shaders);
  Material(Shader_Opacity_Triplet color_shaders, DepthShaderGroup depth_shaders);
  ~Material();

  int draw(Shader::DrawType draw_type, Transparency transparency, int texture_unit);
  void set_opacity_map(Shader* shader, int texture_unit=0);

  Texture load_texture(const char *path, Image_Type type, ImageLoading::Options options=ImageLoading::Options::ADD_TO_MATERIAL);
  Texture load_cubemap(const std::vector<std::string>& faces, bool add_to_material=true);

  bool operator==(const Material& other_material);

  Shader_Opacity_Triplet get_color_shaders() {return color_shaders;}
  DepthShaderGroup get_depth_shaders() {return depth_shaders;}

protected:
  // Helper functions
  void init(Shader_Opacity_Triplet color_shaders, DepthShaderGroup depth_shaders);
  int set_textures(Shader* shader, int texture_unit=0);
};

#endif
