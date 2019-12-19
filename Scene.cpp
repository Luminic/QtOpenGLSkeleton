#include <QDebug>

#include "Scene.h"

std::vector<Texture> Scene::loaded_textures;
std::vector<Material*> Scene::loaded_materials;

Scene::Scene(QObject *parent) : QObject(parent),
  camera(new Camera()),
  sunlight(new PointLight(glm::vec3(1.2f, 0.6, 1.5f), glm::vec3(0.06f))),
  background_color(0.1, 0.1, 0.2)
{
}

Scene::~Scene() {
  delete camera;
  delete sunlight;
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
  shader->setMat4("model", sunlight->get_model_matrix());

  sunlight->draw(shader);
}

void Scene::set_sunlight_settings(std::string name, Shader *shader) {
  shader->use();

  shader->setVec3((name+".direction").c_str(), sunlight->position);

  shader->setVec3((name+".ambient").c_str(), (sunlight->color)*(sunlight->ambient));
  shader->setVec3((name+".diffuse").c_str(), (sunlight->color)*(sunlight->diffuse));
  shader->setVec3((name+".specular").c_str(), (sunlight->color)*(sunlight->specular));
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
