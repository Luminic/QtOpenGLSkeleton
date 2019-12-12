#include "Scene.h"

Scene::Scene() :
  camera(new Camera()),
  sunlight(glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, 1.0f, 1.0f, glm::vec3(1.2f, 0.6, 1.5f), glm::vec3(0.06f)),
  background_color(0.1, 0.1, 0.2)
{
}

Scene::~Scene() {
  delete camera;
}

void Scene::initialize_scene(Shader *shader, unsigned int vbo, unsigned int ebo) {
  initializeOpenGLFunctions();
  sunlight.initialize_shader(shader);
  sunlight.initialize_buffers(vbo, ebo);
}

void Scene::update_scene() {
  camera->update_cam();
}

void Scene::draw_sun(Shader *shader) { // Should be the first thing drawn
  shader->use();
  shader->setMat4("view", glm::lookAt(glm::vec3(0.0f), camera->get_front(), camera->get_up()));
  shader->setMat4("model", sunlight.get_model_matrix());

  glDepthMask(GL_FALSE);
  sunlight.draw();
  glDepthMask(GL_TRUE);
}

void Scene::set_sunlight_settings(std::string name, Shader *shader) {
  shader->use();

  shader->setVec3((name+".direction").c_str(), sunlight.get_position());

  shader->setVec3((name+".ambient").c_str(), sunlight.get_color()*sunlight.get_ambient());
  shader->setVec3((name+".diffuse").c_str(), sunlight.get_color()*sunlight.get_diffuse());
  shader->setVec3((name+".specular").c_str(), sunlight.get_color()*sunlight.get_specular());
}

//void Scene::set_background_color(glm::vec3)
