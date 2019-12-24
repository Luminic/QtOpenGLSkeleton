#include <QDebug>

#include "Sunlight.h"

Sunlight::Sunlight(glm::vec3 polar_position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  Light(glm::vec3(0.0f), scale, color, ambient, diffuse, specular),
  polar_position(polar_position)
{

  x_view_size = 52.0f;
  y_view_size = 35.0f;
  near_plane = 0.1f;
  far_plane = 45.0f;
}

Sunlight::~Sunlight() {}

void Sunlight::set_object_settings(std::string name, Shader *shader) {
  Light::set_object_settings(name, shader);

  shader->setVec3((name+".direction").c_str(), get_position());
}

void Sunlight::initialize_depth_framebuffer(unsigned int depth_map_width, unsigned int depth_map_height) {
  this->depth_map_width = depth_map_width;
  this->depth_map_height = depth_map_height;

  glGenFramebuffers(1, &depth_framebuffer);

  glGenTextures(1, &depth_map);
  glBindTexture(GL_TEXTURE_2D, depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float border_color[] = {1.0f,1.0f,1.0f,1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qDebug() << "INCOMPLETE FRAMEBUFFER!\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 Sunlight::get_position() {
  glm::vec3 pos = glm::vec3(
    glm::sin(glm::radians(polar_position.x))*glm::cos(glm::radians(polar_position.y)),
    glm::sin(glm::radians(polar_position.y)),
    glm::cos(glm::radians(polar_position.x))*glm::cos(glm::radians(polar_position.y))
  );
  return pos * polar_position.z;
}

glm::mat4 Sunlight::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), get_position());
  model = glm::scale(model, glm::vec3(scale));
  return model;
}
