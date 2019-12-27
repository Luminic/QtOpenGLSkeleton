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
#include "Node.h"
#include "Light.h"
#include "Model.h"
#include "Mesh.h"
#include "Scene.h"
#include "Settings.h"

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  OpenGLWindow(QWidget *parent=nullptr);
  ~OpenGLWindow();

  void set_inputs(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time);
  void update_scene();

  Settings *settings;
  Scene *scene;

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

private:
  unsigned int framebuffer;
  unsigned int texture_colorbuffers[2];
  unsigned int renderbuffer;

  unsigned int ping_pong_framebuffer;
  unsigned int ping_pong_colorbuffers[2];
  unsigned int bloom_colorbuffer;

  Mesh *framebuffer_quad;

  float angle;
  glm::mat4 projection;

  Shader *object_shader;
  Shader *light_shader;
  Shader *sunlight_depth_shader;
  Shader *pointlight_depth_shader;
  Shader *skybox_shader;
  Shader *framebuffer_shader;

  Shader *gaussian_blur_shader;

  std::unordered_set<int> *keys_pressed;
  int *delta_time;
  QPoint *mouse_movement;
};

#endif
