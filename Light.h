#ifndef LIGHT_H
#define LIGHT_H

#include <QOpenGLFunctions_4_5_Core>

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Node.h"
#include "Mesh.h"
#include "Shader.h"

class Light : public Node {
  Q_OBJECT

public:
  Light(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  ~Light();

  void set_object_settings(std::string name, Shader *shader);

  // No "override" because only one shader is needed (there is no transparency with light sources)
  virtual void draw(Shader *shader, glm::mat4 model=glm::mat4(1.0f), bool use_material=false, int texture_unit=0);

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
