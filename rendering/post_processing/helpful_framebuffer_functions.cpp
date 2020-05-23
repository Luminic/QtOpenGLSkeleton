#ifndef HELPFUL_FRAMEBUFFER_FUNCTIONS_H
#define HELPFUL_FRAMEBUFFER_FUNCTIONS_H

#include <QOpenGLFunctions_4_5_Core>

// Creates color buffers for the currently bound framebuffers
inline void create_color_buffers(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]) {
  QOpenGLFunctions_4_5_Core* gl_functions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_5_Core>();
  Q_ASSERT_X(gl_functions, "static_load_texture", "Could not get GL functions");

  gl_functions->glGenTextures(nr_colorbuffers, colorbuffers); // generate the colorbuffers

  for (int i=0; i<nr_colorbuffers; i++) {
    gl_functions->glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    gl_functions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    gl_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_functions->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, colorbuffers[i], 0);
  }
  gl_functions->glBindTexture(GL_TEXTURE_2D, 0);

  unsigned int attachments[GL_MAX_COLOR_ATTACHMENTS];
  for (int i=0; i<GL_MAX_COLOR_ATTACHMENTS; i++) {
    attachments[i] = GL_COLOR_ATTACHMENT0+i;
  }

  gl_functions->glDrawBuffers(nr_colorbuffers, attachments);
}

inline void update_color_buffers_size(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]) {
  QOpenGLFunctions_4_5_Core* gl_functions = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_5_Core>();
  Q_ASSERT_X(gl_functions, "static_load_texture", "Could not get GL functions");

  for (int i=0; i<nr_colorbuffers; i++) {
    gl_functions->glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    gl_functions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  gl_functions->glBindTexture(GL_TEXTURE_2D, 0);
}

#endif
