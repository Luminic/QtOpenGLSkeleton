#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Node.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"
#include "Material.h"
#include "Light.h"
#include "PointLight.h"
#include "Sunlight.h"

enum Antialiasing_Types {
  NONE,
  FXAA
};

enum Display_Types {
  SCENE,
  SUNLIGHT_DEPTH,
  POINTLIGHT_DEPTH,
  VOLUMETRICS,
  BLOOM,
  BRIGHT
};

class Scene : public QObject, protected QOpenGLFunctions_4_5_Core {
  Q_OBJECT

public:
  Scene(QObject *parent=nullptr);
  ~Scene();

  void initialize_scene();
  void create_color_buffers(int width, int height, int number, unsigned int colorbuffers[]); // Creates color buffers for the currently bound framebuffer
  void update_color_buffers_size(int width, int height, int number, unsigned int colorbuffers[]); // Resizes the color buffers

  void update_scene();

  void draw_skybox(Shader *shader);
  int set_skybox_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit

  void draw_sun(Shader *shader);
  int set_sunlight_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit

  void render_lights_shadow_map(Shader *shader);
  void draw_light(Shader *shader);
  int set_light_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit

  void draw_objects(Shader *shader, bool use_material, int texture_unit=0);

  static std::vector<Texture> loaded_textures;
  static std::vector<Material*> loaded_materials;

  static Texture is_texture_loaded(std::string image_path); // Returns the texture if the path is already loaded. Returns an empty texture otherwise
  static Material * is_material_loaded(Material *new_material); // Returns new_material if it is unique. Otherwise, it deletes new_material and returns a ptr to the loaded material

  Sunlight *sunlight;
  // PointLight *light;

  std::vector<Node*> objects;
  std::vector<PointLight*> pointlights;

  Mesh *skybox;
  unsigned int skybox_cubemap;


  glm::vec3 background_color;

  bool use_volumetric_lighting;
  float volumetric_lighting_multiplier;
  float volumetric_lighting_offset;
  int volumetric_lighting_steps;
  float henyey_greenstein_G_value;

  float bloom_multiplier;
  float bloom_offset;
  float bloom_threshold_upper;
  float bloom_threshold_lower;
  int bloom_interpolation;
  int bloom_applications;

  Antialiasing_Types antialiasing;

  /*
  Display Types:
    0: Default
    1: Sunlight Depth
    2: Pointlight Depth
    3: Volumetrics
    4: Bloom
  */
  int display_type;

private:
  float angle;
};

#endif
