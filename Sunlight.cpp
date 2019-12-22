#include <QDebug>

#include "Sunlight.h"

Sunlight::Sunlight(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  Light(position, scale, color, ambient, diffuse, specular)
{
}

Sunlight::~Sunlight() {}

void Sunlight::initialize_depth_framebuffer(unsigned int depth_map_width, unsigned int depth_map_height) {
  this->depth_map_width = depth_map_width;
  this->depth_map_height = depth_map_height;

  glGenFramebuffers(1, &depth_framebuffer);

  glGenTextures(1, &depth_map);
  glBindTexture(GL_TEXTURE_2D, depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qDebug() << "INCOMPLETE FRAMEBUFFER!\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
