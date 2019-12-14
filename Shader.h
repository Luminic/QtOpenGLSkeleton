#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QString>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader : public QObject, protected QOpenGLFunctions {
  Q_OBJECT

public:
  unsigned int ID;

  Shader();
  ~Shader();

  void loadShaders(const char *vertex_path, const char *fragment_path);
  void use();

  void setBool(const char *name, bool value);
  void setInt(const char *name, int value);
  void setFloat(const char *name, float value);
  void setVec3(const char *name, const glm::vec3 &vec);
  void setMat4(const char *name, const glm::mat4 &mat);
};

#endif
