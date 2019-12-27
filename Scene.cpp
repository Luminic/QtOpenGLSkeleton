#include <QDebug>

#include "Scene.h"

std::vector<Texture> Scene::loaded_textures;
std::vector<Material*> Scene::loaded_materials;

Scene::Scene(QObject *parent) : QObject(parent) {
  background_color = glm::vec3(0.1, 0.1, 0.2);
  display_type = 0;

  use_volumetric_lighting = false;
  volumetric_lighting_multiplier = 1.0f;
  volumetric_lighting_offset = 0.0f;
  volumetric_lighting_steps = 30;
  henyey_greenstein_G_value = 0.6f;

  camera = new Camera();

  sunlight = new Sunlight(glm::vec3(210.0f, 24.0f, 5.0f), glm::vec3(0.06f));
  sunlight->initialize_depth_framebuffer(2048,2048);
  sunlight->ambient = 1.0f;

  light = new PointLight(glm::vec3(0.4f, 1.6, 2.3f), glm::vec3(0.2f));
  light->initialize_depth_framebuffer(1024,1024);

  cube = new Mesh();
  cube->initialize_cube();
  cube->material = new Material();
  cube->material->load_texture("textures/container2.png", ALBEDO_MAP);
  cube->material->load_texture("textures/container2_specular.png", METALNESS_MAP);
  cube->material->load_texture("textures/container2_specular.png", ROUGHNESS_MAP);
  cube->material->metalness = 1.0f;
  cube->material = Scene::is_material_loaded(cube->material);

  floor = new Node(glm::mat4(1.0f), glm::vec3(0.0f,-3.5f,4.5f), glm::vec3(7.0f,1.0f,7.0f));
  Mesh *floor_mesh = new Mesh();
  floor_mesh->initialize_plane(true, 3.0f);
  floor_mesh->material = new Material();
  floor_mesh->material->load_texture("textures/wood_floor.png", ALBEDO_MAP);
  floor_mesh->material->roughness = 0.65f;
  floor_mesh->material = Scene::is_material_loaded(floor_mesh->material);
  floor->meshes.push_back(floor_mesh);
  floor->scale = glm::vec3(14.0f,1.0f,7.0f);

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  //nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/sphere.fbx");
  nanosuit = new Model("models/lightray_test/wall2.fbx");
  // nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->scale = glm::vec3(0.6f);
  nanosuit->rotation = glm::vec3(180.0f,0.0f,0.0f);
  nanosuit->position = glm::vec3(0.0f,-2.2f,0.0f);

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
  skybox_cubemap = cube->material->load_cubemap(faces, false).id;
}

Scene::~Scene() {
  delete camera;
  delete sunlight;
  delete light;
  delete cube;
  delete floor;
  delete nanosuit;
  delete skybox;

  for (auto m: Scene::loaded_materials)
    delete m;
}

void Scene::initialize_scene() {
}

void Scene::update_scene() {
  camera->update_cam();
}

void Scene::draw_sun(Shader *shader) { // Should be the first thing drawn
  shader->use();
  shader->setMat4("view", glm::lookAt(glm::vec3(0.0f), camera->get_front(), camera->get_up()));
  //shader->setMat4("model", glm::mat4(1.0f));//sunlight->get_model_matrix());

  sunlight->draw(shader);
}

void Scene::set_sunlight_settings(std::string name, Shader *shader, int texture_unit) {
  sunlight->set_object_settings(name, shader);

  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_2D, sunlight->depth_map);
  shader->setInt((name+".shadow_map").c_str(), texture_unit);
}

void Scene::draw_light(Shader *shader) {
  light->draw(shader);
}

void Scene::set_light_settings(std::string name, Shader *shader, int texture_unit) {
  light->set_object_settings((name+"["+std::to_string(0)+"]").c_str(), shader);

  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, light->depth_cubemap);
  shader->setInt((name+"["+std::to_string(0)+"]"+".shadow_cubemap").c_str(), texture_unit);
}

void Scene::draw_skybox(Shader *shader) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
  shader->setInt("skybox", 0);
  skybox->draw(shader);
}

void Scene::set_skybox_settings(std::string name, Shader *shader, int texture_unit) {
  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
  shader->setInt(name.c_str(), texture_unit);
}

void Scene::draw_objects(Shader *shader, bool use_material, int material_index_offset) {
  glm::vec3 cube_positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f,  15.0f),
    glm::vec3(-1.5f, -2.2f,  2.5f),
    glm::vec3(-3.8f, -2.0f,  12.3f),
    glm::vec3( 2.4f, -0.4f,  3.5f),
    glm::vec3(-1.7f,  3.0f,  7.5f),
    glm::vec3( 1.3f, -2.0f,  2.5f),
    glm::vec3( 1.5f,  2.0f,  2.5f),
    glm::vec3( 1.5f,  0.2f,  1.5f),
    glm::vec3(-1.3f,  1.0f,  1.5f)
  };
  for (int i=0; i<10; i++) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cube_positions[i]+glm::vec3(0.0f,0.0f,2.0f));
    model = glm::rotate(model, glm::radians(20.0f*i), glm::vec3(1.0f,0.3f,0.5f));
    model = glm::scale(model, glm::vec3(1.0f));//cube->get_scale());
    shader->setMat4("model", model);
    cube->draw(shader, use_material, material_index_offset);
  }

  // Render the floor
  floor->draw(shader, glm::mat4(1.0f), use_material, material_index_offset);

  // Render the Nanosuit
  glm::mat4 model = nanosuit->get_model_matrix();
  nanosuit->draw(shader, model, use_material, material_index_offset);
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

//void Scene::set_background_color(glm::vec3)
