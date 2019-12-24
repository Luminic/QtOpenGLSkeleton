#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "Light.h"

class PointLight : public Light {
  Q_OBJECT

public:
  PointLight(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f, float constant=1.0f, float linear=0.09f, float quadratic=0.032f);
  ~PointLight();

  void set_object_settings(std::string name, Shader *shader);

  // Falloff
  float constant;
  float linear;
  float quadratic;

};

#endif
