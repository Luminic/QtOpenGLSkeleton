#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "Shader.h"

class Light : public Mesh {
  Q_OBJECT

public:
  Light(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  ~Light();

  void set_object_settings(std::string name, Shader *shader);

  void draw(Shader *shader);

  glm::vec3 position;
  glm::vec3 scale;

  glm::vec3 color;
  float ambient;
  float diffuse;
  float specular;

  // Falloff
  float constant;
  float linear;
  float quadratic;

  // Getters
  virtual glm::mat4 get_model_matrix();
};

#endif
