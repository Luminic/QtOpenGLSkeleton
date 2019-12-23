#ifndef SUNLIGHT_H
#define SUNLIGHT_H

#include "Light.h"

class Sunlight : public Light {
  Q_OBJECT

public:
  Sunlight(glm::vec3 position_euler_angles=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  ~Sunlight();

  void initialize_depth_framebuffer(unsigned int depth_map_width=1024, unsigned int depth_map_height=1024);

  glm::vec3 get_position();
  glm::mat4 get_model_matrix();

  // X & Y are yaw and pitch on a sphere. Z is the radius of the sphere
  glm::vec3 polar_position;

  unsigned int depth_framebuffer;
  unsigned int depth_map;

  unsigned int depth_map_width;
  unsigned int depth_map_height;

  float x_view_size;
  float y_view_size;
  float near_plane;
  float far_plane;
};

#endif
