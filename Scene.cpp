#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QDebug>

#include "Scene.h"

std::vector<Texture> Scene::loaded_textures;
std::vector<Material*> Scene::loaded_materials;

Scene::Scene(QObject *parent) : QObject(parent) {
  initializeOpenGLFunctions();

  background_color = glm::vec3(0.1, 0.1, 0.2);
  display_type = 0;

  use_volumetric_lighting = false;
  volumetric_lighting_multiplier = 1.0f;
  volumetric_lighting_offset = 0.0f;
  volumetric_lighting_steps = 30;
  henyey_greenstein_G_value = 0.6f;

  bloom_multiplier = 0.2f;
  bloom_offset = 0.0f;
  bloom_threshold_upper = 1.2f;
  bloom_threshold_lower = 0.7f;
  bloom_interpolation = 1;
  bloom_applications = 10;

  sunlight = new Sunlight(glm::vec3(210.0f, 24.0f, 5.0f), glm::vec3(0.06f));
  sunlight->initialize_depth_framebuffer(2048,2048);
  sunlight->ambient = 0.5f;
  sunlight->diffuse = 2.5f;
  sunlight->specular = 2.5f;

  Node* floor = new Node(glm::mat4(1.0f), glm::vec3(0.0f,-3.5f,4.5f), glm::vec3(7.0f,1.0f,7.0f));
  Mesh* floor_mesh = new Mesh();
  floor_mesh->initialize_plane(true, 3.0f);
  floor_mesh->material = new Material();
  floor_mesh->material->load_texture("textures/wood_floor.png", ALBEDO_MAP);
  floor_mesh->material->ambient = 0.2f;
  floor_mesh->material->diffuse = 0.6f;
  floor_mesh->material->specular = 0.3f;
  floor_mesh->material->roughness = 0.66f;
  floor_mesh->material = Scene::is_material_loaded(floor_mesh->material);

  floor->meshes.push_back(std::shared_ptr<Mesh>(floor_mesh));
  floor->scale = glm::vec3(14.0f,1.0f,7.0f);
  add_node(std::shared_ptr<Node>(floor));

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  // nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/sphere.fbx");
  // Model* nanosuit = new Model("models/lightray_test/wall2.fbx");
  Model* nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->scale = glm::vec3(0.3f);
  nanosuit->rotation = glm::vec3(180.0f,0.0f,0.0f);
  nanosuit->position = glm::vec3(0.0f,-3.5f,0.0f);
  add_node(std::shared_ptr<Node>(nanosuit));

  skybox = new Mesh();
  skybox->initialize_cube();
  std::vector<std::string> faces {
    "skyboxes/right.jpg",
    "skyboxes/left.jpg",
    "skyboxes/top.jpg",
    "skyboxes/bottom.jpg",
    "skyboxes/front.jpg",
    "skyboxes/back.jpg"
  };
  skybox->material = new Material();
  skybox->material->load_cubemap(faces);

  antialiasing = FXAA;
}

Scene::~Scene() {
  delete sunlight;
  delete skybox;

  for (auto m: Scene::loaded_materials)
    delete m;
}

void Scene::initialize_scene() {
}

void Scene::create_color_buffers(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]) {
  glGenTextures(nr_colorbuffers, colorbuffers); // generate the colorbuffers

  for (int i=0; i<nr_colorbuffers; i++) {
    glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, colorbuffers[i], 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  unsigned int attachments[GL_MAX_COLOR_ATTACHMENTS];
  for (int i=0; i<GL_MAX_COLOR_ATTACHMENTS; i++) {
    attachments[i] = GL_COLOR_ATTACHMENT0+i;
  }

  glDrawBuffers(nr_colorbuffers, attachments);
}

void Scene::update_color_buffers_size(int width, int height, int nr_colorbuffers, unsigned int colorbuffers[]) {
  for (int i=0; i<nr_colorbuffers; i++) {
    glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::update_scene() {

}

void Scene::draw_skybox(Shader *shader) {
  skybox->draw(shader);
}

int Scene::set_skybox_settings(std::string name, Shader *shader, int texture_unit) {
  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->material->textures[0].id); // The skybox should only have 1 texture
  shader->setInt(name.c_str(), texture_unit);

  texture_unit++;
  return texture_unit;
}

void Scene::render_suns_shadow_map(Shader *shader, glm::mat4& sun_space) {
  sunlight->bind_sunlight_framebuffer(shader, sun_space);
  draw_objects(shader, false);
}

int Scene::set_sunlight_settings(std::string name, Shader* shader, int texture_unit) {
  sunlight->set_object_settings(name, shader);

  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_2D, sunlight->depth_map);
  shader->setInt((name+".shadow_map").c_str(), texture_unit);
  texture_unit++;

  return texture_unit;
}

void Scene::draw_sun(Shader *shader) { // Should be the first thing drawn
  sunlight->draw(shader);
}

void Scene::render_pointlights_shadow_map(Shader *shader) {
  for (auto light : pointlights) {
    light->bind_pointlight_framebuffer(shader);
    int texture_unit = 0;
    draw_objects(shader, false, texture_unit);
  }
}

int Scene::set_light_settings(std::string name, Shader* shader, int texture_unit) {
  shader->setInt("nr_lights", pointlights.size());
  for (unsigned int i=0; i<pointlights.size(); i++) {
    pointlights[i]->set_object_settings((name+"["+std::to_string(i)+"]").c_str(), shader);

    glActiveTexture(GL_TEXTURE0+texture_unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pointlights[i]->depth_cubemap);
    shader->setInt((name+"["+std::to_string(i)+"]"+".shadow_cubemap").c_str(), texture_unit);
    texture_unit++;
  }

  return texture_unit;
}

void Scene::draw_light(Shader *shader) {
  for (auto light : pointlights) {
    light->draw(shader);
  }
}

void Scene::draw_objects(Shader *shader, bool use_material, int texture_unit) {
  for (auto node : nodes) {
    node->draw(shader, glm::mat4(1.0f), use_material, texture_unit);
  }
}

Texture Scene::is_texture_loaded(std::string image_path) {
  for (unsigned int i=0; i<loaded_textures.size(); i++) {
    if (image_path == Scene::loaded_textures[i].path) {
      return Scene::loaded_textures[i];
    }
  }
  return Texture();
}

Material * Scene::is_material_loaded(Material *new_material) {
  for (auto m : Scene::loaded_materials) {
    if ((*new_material) == (*m)) {
      delete new_material;
      return m;
    }
  }
  Scene::loaded_materials.push_back(new_material);
  return new_material;
}

// Getters and Setters
void Scene::add_node(std::shared_ptr<Node> node) {
  nodes.push_back(node);
}

void Scene::delete_node_at(unsigned int index) {
  Q_ASSERT_X(index < nodes.size(), "delete_node_at", "index is greater than vector nodes' size");
  nodes.erase(nodes.begin() + index);
}

void Scene::clear_nodes() {
  nodes.clear();
}

void Scene::add_pointlight(std::shared_ptr<PointLight> pointlight) {
  pointlights.push_back(pointlight);
}

void Scene::delete_pointlight_at(unsigned int index) {
  Q_ASSERT_X(index < pointlights.size(), "delete_pointlight_at", "index is greater than vector pointlights' size");
  pointlights.erase(pointlights.begin() + index);
}

void Scene::clear_pointlights() {
  pointlights.clear();
}

//void Scene::set_background_color(glm::vec3)
