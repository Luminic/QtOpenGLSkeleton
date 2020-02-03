#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QWheelEvent>

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

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  OpenGLWindow(QWidget *parent=nullptr);
  ~OpenGLWindow();

  void set_inputs(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time);
  void update_scene();

  void update_perspective_matrix();

  Settings *settings;
  Scene *scene;

  float fov;

protected:
  void initializeGL() override;
  void load_shaders();
  void create_framebuffer();
  void create_scene_framebuffer();
  void create_ping_pong_framebuffer();
  void create_post_processing_framebuffer();

  void paintGL() override;
  void resizeGL(int w, int h) override;

  void wheelEvent(QWheelEvent *event) override;

private:
  unsigned int framebuffer;
  unsigned int renderbuffer;
  unsigned int colorbuffers[2];

  unsigned int scene_framebuffer;
  unsigned int scene_colorbuffers[2];

  unsigned int ping_pong_framebuffer;
  unsigned int ping_pong_colorbuffers[2]; // Half Resolution
  unsigned int bloom_colorbuffer; // Half Resolution

  unsigned int post_processing_framebuffer;
  unsigned int post_processing_colorbuffer;

  Mesh *framebuffer_quad;

  float angle;
  glm::mat4 projection;

  Shader *sunlight_depth_shader;
  Shader *pointlight_depth_shader;

  Shader *object_shader;
  Shader *light_shader;
  Shader *skybox_shader;
  Shader *scene_shader;

  Shader *gaussian_blur_shader;
  Shader *post_processing_shader;
  Shader *antialiasing_shader;

  std::unordered_set<int> *keys_pressed;
  int *delta_time;
  QPoint *mouse_movement;
};

#endif
