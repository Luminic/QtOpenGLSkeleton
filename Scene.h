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
#include "DirectionalLight.h"

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
  void create_color_buffers(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]); // Creates color buffers for the currently bound framebuffer
  void update_color_buffers_size(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]); // Resizes the color buffers

  void update_scene();

  void draw_skybox(Shader *shader);
  int set_skybox_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit

  void render_dirlights_shadow_map(Shader *shader);
  int set_dirlight_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit
  void draw_dirlight(Shader *shader);

  void render_pointlights_shadow_map(Shader *shader);
  int set_light_settings(std::string name, Shader *shader, int texture_unit=0); // Returns the next free texture unit
  void draw_light(Shader *shader);

  void draw_objects(Shader *shader, bool use_material, int texture_unit=0);

  static std::vector<Texture> loaded_textures;
  static std::vector<Material*> loaded_materials;

  static Texture is_texture_loaded(std::string image_path); // Returns the texture if the path is already loaded. Returns an empty texture otherwise
  static Material * is_material_loaded(Material *new_material); // Returns new_material if it is unique. Otherwise, it deletes new_material and returns a ptr to the loaded material

  // Getters and Setters
  const std::vector<std::shared_ptr<Node>>& get_nodes() const {return nodes;}
  unsigned int nodes_size() const {return nodes.size();}
  std::shared_ptr<Node> get_node_at(unsigned int index) {return nodes[index];}
  void add_node(std::shared_ptr<Node> node);
  void delete_node_at(unsigned int index);
  void clear_nodes();

  const std::vector<std::shared_ptr<DirectionalLight>>& get_dirlights() const {return dirlights;}
  void add_dirlight(std::shared_ptr<DirectionalLight> dirlight);
  void delete_dirlight_at(unsigned int index);
  void clear_dirlights();

  const std::vector<std::shared_ptr<PointLight>>& get_pointlights() const {return pointlights;}
  void add_pointlight(std::shared_ptr<PointLight> pointlight);
  void delete_pointlight_at(unsigned int index);
  void clear_pointlights();

  Mesh *skybox;

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

protected:
  std::vector<std::shared_ptr<Node>> nodes;
  std::vector<std::shared_ptr<DirectionalLight>> dirlights;
  std::vector<std::shared_ptr<PointLight>> pointlights;

private:
  float angle;
};

#endif
