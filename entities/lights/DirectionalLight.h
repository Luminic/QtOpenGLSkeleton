#ifndef DIRLIGHT_H
#define DIRLIGHT_H

#include "Light.h"

class DirectionalLight : public Light {
  Q_OBJECT

public:
  DirectionalLight(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  virtual ~DirectionalLight();

  void set_object_settings(std::string name, Shader *shader);

  void initialize_depth_framebuffer(unsigned int depth_map_width=1024, unsigned int depth_map_height=1024);
  void bind_dirlight_framebuffer();
  void set_light_space(Shader* depth_shader);

  glm::mat4 get_model_matrix(bool use_transformation_matrix=true) override;

  glm::vec3 get_direction();
  void set_direction(float yaw, float pitch);
  void set_direction(glm::vec3 dir);

  unsigned int depth_framebuffer;
  unsigned int depth_map;

  unsigned int depth_map_width;
  unsigned int depth_map_height;

  glm::mat4 dirlight_space;
  float x_view_size;
  float y_view_size;
  float near_plane;
  float far_plane;

  glm::vec3 direction;

  // Getters and setters
  // Disable the setting of scale and rotation
  virtual void set_scale(glm::vec3 sca) override;
  virtual void set_rotation(glm::vec3 rot) override;
};

#endif
