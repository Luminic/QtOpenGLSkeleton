#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "../../entities/meshes/Mesh.h"
#include "../Shader.h"

class GaussianBlur: public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT;

public:
  // Can be called before OpenGL functions are initialized
  GaussianBlur();
  ~GaussianBlur();

  // Must be called before apply_blur is used but after OpenGL functions are initialized
  // GaussianBlur will not handle the memory management of framebuffer_quad
  void init(Mesh* framebuffer_quad);

  unsigned int apply_blur(unsigned int source_colorbuffer, int strength, int resulting_width, int resulting_height);

private:
  Mesh* framebuffer_quad;
  Shader* gaussian_blur_shader = nullptr; // set to nullptr in case init is never called

  int current_framebuffer_width;
  int current_framebuffer_height;

  unsigned int ping_pong_framebuffer;
  unsigned int ping_pong_colorbuffers[2];
};

#endif
