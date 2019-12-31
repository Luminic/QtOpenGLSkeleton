#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Node.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Material.h"
#include "Light.h"
#include "PointLight.h"
#include "Sunlight.h"

class Scene : public QObject {
  Q_OBJECT

public:
  Scene(QObject *parent=nullptr);
  ~Scene();
  void initialize_scene();

  void update_scene();

  void draw_sun(Shader *shader);
  void set_sunlight_settings(std::string name, Shader *shader, int texture_unit=0);
  void draw_light(Shader *shader);
  void set_light_settings(std::string name, Shader *shader, int texture_unit=0);
  void draw_skybox(Shader *shader);
  void set_skybox_settings(std::string name, Shader *shader, int texture_unit=0);
  void draw_objects(Shader *shader, bool use_material=false, int material_index_offset=0);

  Camera *camera;
  Sunlight *sunlight; // Display it like a pointlight but have its lighting be sunlight
  PointLight *light;

  Mesh *cube;
  Node *floor;
  Model *nanosuit;

  Mesh *skybox;
  unsigned int skybox_cubemap;

  static std::vector<Texture> loaded_textures;
  static Texture is_texture_loaded(std::string image_path); // Returns the texture if the path is already loaded. Returns an empty texture otherwise

  static std::vector<Material*> loaded_materials;
  static Material * is_material_loaded(Material *new_material); // Returns new_material if it is unique. Otherwise, it deletes new_material and returns a ptr to the loaded material

  glm::vec3 background_color;

  bool use_volumetric_lighting;
  float volumetric_lighting_multiplier;
  float volumetric_lighting_offset;
  int volumetric_lighting_steps;
  float henyey_greenstein_G_value;

  int display_type;

private:
  float angle;
};

#endif
