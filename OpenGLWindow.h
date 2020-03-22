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

  Camera *camera = nullptr;

  Settings *settings = nullptr;
  Scene *scene = nullptr;

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

  Mesh* framebuffer_quad = nullptr;

  glm::mat4 projection;

  Shader_Opacity_Triplet dirlight_depth_shaders;
  Shader_Opacity_Triplet pointlight_depth_shaders;

  Shader_Opacity_Triplet object_shaders;

  Shader *light_shader = nullptr;
  Shader *skybox_shader = nullptr;
  Shader *scene_shader = nullptr;

  Shader *gaussian_blur_shader = nullptr;
  Shader *post_processing_shader = nullptr;
  Shader *antialiasing_shader = nullptr;

  std::unordered_set<int> *keys_pressed = nullptr;
  int* delta_time = nullptr;
  QPoint* mouse_movement = nullptr;
};

#endif
