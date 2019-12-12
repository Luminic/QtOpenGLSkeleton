#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QOpenGLFunctions_4_2_Core>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "PointLight.h"

class Scene : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Scene();
  ~Scene();
  void initialize_scene(Shader *shader, unsigned int vbo, unsigned int ebo);

  void update_scene();

  void draw_sun(Shader *shader);
  void set_sunlight_settings(std::string name, Shader *shader);

  Camera *camera;
  PointLight sunlight; // Display it like a pointlight but have its lighting be sunlight

  glm::vec3 background_color;

private:
  float angle;
};

#endif
