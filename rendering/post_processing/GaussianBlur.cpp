#include "GaussianBlur.h"
#include "helpful_framebuffer_functions.cpp"

#include <QDebug>

GaussianBlur::GaussianBlur() {}

void GaussianBlur::init(Mesh* framebuffer_quad) {
  this->framebuffer_quad = framebuffer_quad;
  initializeOpenGLFunctions();

  glGenFramebuffers(1, &ping_pong_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);

  // These have to be created seperately because only one will be drawn on at a time
  create_color_buffers(400, 300, 1, &ping_pong_colorbuffers[0]);
  create_color_buffers(400, 300, 1, &ping_pong_colorbuffers[1]);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  gaussian_blur_shader = new Shader();
  gaussian_blur_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/gaussian_blur_fragment.shader");
  gaussian_blur_shader->validate_program();
}

GaussianBlur::~GaussianBlur() {
  delete gaussian_blur_shader;
}

unsigned int GaussianBlur::apply_blur(unsigned int source_colorbuffer, int strength, int resulting_width, int resulting_height) {
  // Create bloom effect with gaussian blur
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);

  if (resulting_width!=current_framebuffer_width || resulting_height!=current_framebuffer_height) {
    current_framebuffer_width = resulting_width;
    current_framebuffer_height = resulting_height;
    update_color_buffers_size(current_framebuffer_width, current_framebuffer_height, 2, ping_pong_colorbuffers);
  }

  glViewport(0, 0, resulting_width, resulting_height);

  // Clear the buffers (if they aren't cleared it causes problems when window is resized)
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[1], 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[0], 0);
  glClear(GL_COLOR_BUFFER_BIT);

  gaussian_blur_shader->use();

  bool horizontal=true;
  for (int i=0; i<strength*2; i++) {
    gaussian_blur_shader->setBool("horizontal", horizontal);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[i%2], 0);

    glActiveTexture(GL_TEXTURE0);
    if (i==0) {
      glBindTexture(GL_TEXTURE_2D, source_colorbuffer);
    } else {
      glBindTexture(GL_TEXTURE_2D, ping_pong_colorbuffers[(i+1)%2]);
    }
    gaussian_blur_shader->setInt("image", 0);

    framebuffer_quad->simple_draw();

    horizontal = !horizontal;
  }
  return ping_pong_colorbuffers[1];
}
