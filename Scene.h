#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QOpenGLFunctions_4_2_Core>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Material.h"
#include "PointLight.h"

class Scene : public QObject {
  Q_OBJECT

public:
  Scene(QObject *parent=nullptr);
  ~Scene();
  void initialize_scene();

  void update_scene();

  void draw_sun(Shader *shader);
  void set_sunlight_settings(std::string name, Shader *shader);

  Camera *camera;
  PointLight *sunlight; // Display it like a pointlight but have its lighting be sunlight

  static std::vector<Texture> loaded_textures;
  static Texture is_texture_loaded(std::string image_path); // Returns the texture if the path is already loaded. Returns an empty texture otherwise

  static std::vector<Material*> loaded_materials;
  static Material * is_material_loaded(Material *new_material); // Returns new_material if it is unique. Otherwise, it deletes new_material and returns a ptr to the loaded material

  glm::vec3 background_color;

private:
  float angle;
};

#endif
