#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QString>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  unsigned int ID;

  Shader();
  ~Shader();

  void loadShaders(const char *vertex_path, const char *fragment_path, const char *geometry_path="");
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
};

#endif
