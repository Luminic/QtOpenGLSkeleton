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

  bloom_multiplier = 0.5f;
  bloom_offset = 0.0f;
  bloom_threshold_upper = 1.2f;
  bloom_threshold_lower = 0.5f;
  bloom_interpolation = 1;
  bloom_applications = 10;

  sunlight = new Sunlight(glm::vec3(210.0f, 24.0f, 5.0f), glm::vec3(0.06f));
  sunlight->initialize_depth_framebuffer(2048,2048);
  sunlight->ambient = 2.5f;
  sunlight->diffuse = 3.0f;
  sunlight->specular = 3.0f;

  light = new PointLight(glm::vec3(0.4f, 1.6, 2.3f), glm::vec3(0.2f));
  light->initialize_depth_framebuffer(1024,1024);
  light->color = glm::vec3(1.3f);

  std::shared_ptr<Mesh> cube = std::make_shared<Mesh>();
  cube->initialize_cube();
  cube->material = new Material();
  cube->material->load_texture("textures/container2.png", ALBEDO_MAP);
  cube->material->load_texture("textures/container2_specular.png", METALNESS_MAP);
  cube->material->load_texture("textures/container2_specular.png", ROUGHNESS_MAP);
  cube->material->metalness = 1.0f;
  cube->material = Scene::is_material_loaded(cube->material);

  glm::vec3 cube_positions[10] = {
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
  for (int i=0; i<1; i++) {
    Node* n = new Node(glm::mat4(1.0f), cube_positions[i], glm::vec3(1.0f), glm::vec3(3.2f*i,0.6f*i,-2.0f*i));
    n->meshes.push_back((cube));
    objects.push_back(n);
  }

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
  objects.push_back(floor);

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  // nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/sphere.fbx");
  // nanosuit = new Model("models/lightray_test/wall2.fbx");
  Model* nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->scale = glm::vec3(0.3f);
  nanosuit->rotation = glm::vec3(180.0f,0.0f,0.0f);
  nanosuit->position = glm::vec3(0.0f,-3.5f,0.0f);
  objects.push_back(nanosuit);

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

  antialiasing = FXAA;
}

Scene::~Scene() {
  delete sunlight;
  delete light;
  // delete cube;
  // delete floor;
  // delete nanosuit;
  delete skybox;

  for (auto object : objects)
    delete object;

  for (auto m: Scene::loaded_materials)
    delete m;
}

void Scene::initialize_scene() {
}

void Scene::create_color_buffers(int width, int height, int number, unsigned int colorbuffers[]) {
  glGenTextures(number, colorbuffers); // generate the colorbuffers

  for (int i=0; i<number; i++) {
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

  glDrawBuffers(number, attachments);
}

void Scene::update_color_buffers_size(int width, int height, int number, unsigned int colorbuffers[]) {
  for (int i=0; i<number; i++) {
    glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Scene::update_scene() {

}

void Scene::draw_sun(Shader *shader) { // Should be the first thing drawn
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
  /*glm::vec3 cube_positions[] = {
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
  }*/

  // Render the floor
  // floor->draw(shader, glm::mat4(1.0f), use_material, material_index_offset);
  for (Node* object : objects) {
    object->draw(shader, glm::mat4(1.0f), use_material, material_index_offset);
  }

  // Render the Nanosuit
  // glm::mat4 model = nanosuit->get_model_matrix();
  // nanosuit->draw(shader, glm::mat4(1.0f), use_material, material_index_offset);
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
