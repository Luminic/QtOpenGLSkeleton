#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QString>

#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum Image_Type {
  UNKNOWN =               0x0,
  ALBEDO_MAP =            0x1, // Should be in gamma space (will be converted into linear space)
  AMBIENT_OCCLUSION_MAP = 0x1 << 1, // Should be in linear space
  ROUGHNESS_MAP =         0x1 << 2, // Should be in linear space
  METALNESS_MAP =         0x1 << 3, // Should be in linear space
  CUBE_MAP =              0x1 << 4, // Should be in gamma space (will be converted into linear space)
  OPACITY_MAP =           0x1 << 5 // Only used when a mesh w/ transparent arts is drawn. Only the alpha value is used
};
inline constexpr Image_Type operator|(Image_Type a, Image_Type b) {
  return a = static_cast<Image_Type> (int(a) | int(b));
}
inline constexpr Image_Type operator&(Image_Type a, Image_Type b) {
  return a = static_cast<Image_Type> (int(a) & int(b));
}

class Shader : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT;

public:
  static unsigned int placeholder_texture;
  static std::unordered_map<std::string, unsigned int> uniform_block_buffers;

  enum DrawType {
    COLOR = 0x0,
    DEPTH_DIRLIGHT = 0x1,
    DEPTH_POINTLIGHT = 0x2
  };

  unsigned int ID;

  Shader();
  ~Shader();

  void loadShaders(const char* vertex_path, const char* fragment_path, const char* geometry_path="");
  void initialize_placeholder_textures(Image_Type texture_types); // Input the types of images that the shaders have samplers for
  void initialize_placeholder_2D_textures(std::vector<const char*> texture_names);
  bool validate_program(); // Will return whether or not the program is valid. If invalid, a warning will be outputted to stdout
  void use();

  void setBool(const char *name, bool value);
  void setInt(const char *name, int value);
  void setFloat(const char *name, float value);
  void setVec3(const char *name, const glm::vec3 &vec);
  void setMat4(const char *name, const glm::mat4 &mat);
};

struct Shader_Opacity_Triplet {
  Shader* opaque = nullptr;
  Shader* full_transparency = nullptr;
  Shader* partial_transparency = nullptr;

  void delete_shaders() {
    delete opaque;
    delete full_transparency;
    delete partial_transparency;
  }

  bool validate_shader_programs() {
    return (
      opaque->validate_program() &&
      full_transparency->validate_program() &&
      partial_transparency->validate_program()
    );
  }

  void setBool(const char *name, bool value) {
    opaque->use();
    opaque->setBool(name, value);
    full_transparency->use();
    full_transparency->setBool(name, value);
    partial_transparency->use();
    partial_transparency->setBool(name, value);
  }
  void setInt(const char *name, int value) {
    opaque->use();
    opaque->setInt(name, value);
    full_transparency->use();
    full_transparency->setInt(name, value);
    partial_transparency->use();
    partial_transparency->setInt(name, value);
  }
  void setFloat(const char *name, float value) {
    opaque->use();
    opaque->setFloat(name, value);
    full_transparency->use();
    full_transparency->setFloat(name, value);
    partial_transparency->use();
    partial_transparency->setFloat(name, value);
  }
  void setVec3(const char *name, const glm::vec3 &vec){
    opaque->use();
    opaque->setVec3(name, vec);
    full_transparency->use();
    full_transparency->setVec3(name, vec);
    partial_transparency->use();
    partial_transparency->setVec3(name, vec);
  }
  void setMat4(const char *name, const glm::mat4 &mat){
    opaque->use();
    opaque->setMat4(name, mat);
    full_transparency->use();
    full_transparency->setMat4(name, mat);
    partial_transparency->use();
    partial_transparency->setMat4(name, mat);
  }
};

inline bool operator==(const Shader_Opacity_Triplet& lhs, const Shader_Opacity_Triplet& rhs) {
  return lhs.opaque == rhs.opaque && lhs.full_transparency == rhs.full_transparency && lhs.partial_transparency == rhs.partial_transparency;
}
inline bool operator<(const Shader_Opacity_Triplet& lhs, const Shader_Opacity_Triplet& rhs) {
  if (lhs.opaque->ID < rhs.opaque->ID) return true;
  if (lhs.opaque->ID > rhs.opaque->ID) return false;
  if (lhs.full_transparency->ID < rhs.full_transparency->ID) return true;
  if (lhs.full_transparency->ID > rhs.full_transparency->ID) return false;
  if (lhs.partial_transparency->ID < rhs.partial_transparency->ID) return true;
  return false;
}
inline bool operator>(const Shader_Opacity_Triplet& lhs, const Shader_Opacity_Triplet& rhs) {
  if (lhs < rhs) return false;
  if (lhs == rhs) return false;
  return true;
}

struct DepthShaderGroup {
  Shader_Opacity_Triplet dirlight;
  Shader_Opacity_Triplet pointlight;
};

inline bool operator==(const DepthShaderGroup& lhs, const DepthShaderGroup& rhs) {
  return lhs.dirlight == rhs.dirlight && lhs.pointlight == rhs.pointlight;
}
inline bool operator<(const DepthShaderGroup& lhs, const DepthShaderGroup& rhs) {
  if (lhs.dirlight < rhs.dirlight) return true;
  if (lhs.dirlight > rhs.dirlight) return false;
  if (lhs.pointlight < rhs.pointlight) return true;
  return false;
}

#endif
