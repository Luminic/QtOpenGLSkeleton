#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLWidget>

#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Object.h"
#include "PointLight.h"
#include "Model.h"
#include "Mesh.h"
#include "Scene.h"
#include "Settings.h"

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  OpenGLWindow(QWidget *parent=nullptr);
  ~OpenGLWindow();

  void update_scene();

  Settings *settings;
  Scene *scene;
  Object object;
  PointLight light;

  Model *nanosuit;

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

private:
  float angle;
  glm::mat4 projection;

  Shader *object_shader;
  Shader *light_shader;
};

#endif
