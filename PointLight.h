#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Shader.h"

class PointLight : public Mesh {
  Q_OBJECT

public:
  PointLight(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  ~PointLight();

  void set_object_settings(std::string name, Shader *shader);

  void draw(Shader *shader);

  glm::vec3 position;
  glm::vec3 scale;

  // Getters
  glm::mat4 get_model_matrix();
  glm::vec3 get_color();
  float get_ambient();
  float get_diffuse();
  float get_specular();
  float get_constant();
  float get_linear();
  float get_quadratic();

public slots:
  // Setters
  void set_falloff(float constant, float linear, float quadratic);
  void set_color(glm::vec3 color);
  void set_red(float red);
  void set_green(float green);
  void set_blue(float blue);
  void set_ambient(float ambient);
  void set_diffuse(float diffuse);
  void set_specular(float specular);
  void set_constant(float constant);
  void set_linear(float linear);
  void set_quadratic(float quadratic);

private:
  glm::vec3 color;
  float ambient;
  float diffuse;
  float specular;

  // Falloff
  float constant;
  float linear;
  float quadratic;
};

#endif
