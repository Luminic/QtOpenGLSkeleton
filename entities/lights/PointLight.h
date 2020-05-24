#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"

class PointLight : public Light {
  Q_OBJECT

public:
  PointLight(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=1.0f, float diffuse=1.0f, float specular=1.0f, float constant=1.0f, float linear=0.09f, float quadratic=0.032f);
  virtual ~PointLight();

  void set_object_settings(std::string name, Shader *shader);

  void initialize_depth_framebuffer(unsigned int depth_map_width, unsigned int depth_map_height);
  void bind_pointlight_framebuffer();
  void set_light_space(Shader *depth_shader);

  unsigned int depth_framebuffer;
  unsigned int depth_cubemap;

  unsigned int depth_map_width;
  unsigned int depth_map_height;

  glm::mat4 pointlight_views[6];
  float near_plane;
  float far_plane;

  int samples;
  float sample_radius;

  float constant;
  float linear;
  float quadratic;

};

#endif
