#ifndef OPENGL_WINDOW_H
#define OPENGL_WINDOW_H

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QWheelEvent>

#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/Shader.h"
#include "rendering/Camera.h"
#include "rendering/Scene.h"
#include "rendering/post_processing/GaussianBlur.h"
#include "entities/nodes/Node.h"
#include "entities/nodes/Model.h"
#include "entities/lights/Light.h"
#include "entities/meshes/Mesh.h"
#include "entities/meshes/shapes/Tesseract.h"
#include "utility/Settings.h"

class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  OpenGLWindow(QWidget* parent=nullptr);
  ~OpenGLWindow();

  void set_inputs(const std::unordered_set<int>* keys_pressed, const QPoint* mouse_movement, const int* delta_time);
  void update_scene();

  void update_perspective_matrix();

  Camera camera;

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

  std::shared_ptr<Tesseract> tesseract;

  unsigned int framebuffer;
  unsigned int renderbuffer;
  unsigned int colorbuffers[2];

  unsigned int scene_framebuffer;
  unsigned int scene_colorbuffers[2];

  unsigned int post_processing_framebuffer;
  unsigned int post_processing_colorbuffer;

  Mesh* framebuffer_quad = nullptr;

  glm::mat4 projection;

  Shader_Opacity_Triplet object_shaders;
  DepthShaderGroup depth_shaders;

  Shader *light_shader = nullptr;
  Shader *skybox_shader = nullptr;
  Shader *scene_shader = nullptr;

  GaussianBlur gaussian_blur;

  Shader *post_processing_shader = nullptr;
  Shader *antialiasing_shader = nullptr;

  const std::unordered_set<int>* keys_pressed = nullptr;
  const QPoint* mouse_movement = nullptr;
  const int* delta_time = nullptr;
};

#endif
