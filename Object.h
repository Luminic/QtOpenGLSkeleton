#ifndef OBJECT_H
#define OBJECT_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_2_Core>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

struct Material {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
};

class Object : public QObject, protected QOpenGLFunctions_4_2_Core {
  Q_OBJECT

public:
  Object(Material material, glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f));
  Object();
  ~Object();

  void initialize_shader(Shader *shader);
  void initialize_buffers();
  void initialize_buffers(unsigned int vbo, unsigned int ebo);
  void load_texture(const char *image_path, const char *image_name);

  void draw();

  Shader *shader;

  // Getters
  glm::mat4 get_model_matrix();
  glm::vec3 get_position();
  glm::vec3 get_scale();
  float get_shininess();
  unsigned int get_VBO();
  unsigned int get_EBO();

public slots:
  // Setters
  void set_position(glm::vec3 position);
  void set_x_position(float x);
  void set_y_position(float y);
  void set_z_position(float z);
  void set_scale(glm::vec3 scale);
  void set_x_scale(float x);
  void set_y_scale(float y);
  void set_z_scale(float z);
  void set_shininess(float shininess);

protected:
  std::vector<unsigned int> textures;
  Material material;

  glm::vec3 position;
  glm::vec3 scale;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
};

#endif
