#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QDebug>

#include <algorithm>

#include <glm/gtx/norm.hpp>

#include "Scene.h"

std::vector<Texture> Scene::loaded_textures;
std::vector<Material*> Scene::loaded_materials;

Scene::Scene(QObject *parent) : QObject(parent) {
  initializeOpenGLFunctions();

  display_type = 0;

  skybox_multiplier = 3.5f;

  bloom_multiplier = 0.2f;
  bloom_offset = 0.0f;
  bloom_threshold_upper = 4.0f;
  bloom_threshold_lower = 2.5f;
  bloom_interpolation = 1;
  bloom_applications = 10;

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
  skybox->material = new Material("skybox");
  skybox->material->load_cubemap(faces);

  antialiasing = FXAA;
}

Scene::~Scene() {
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
  for (auto node : nodes) {
    node->update();
  }
}

void Scene::draw_skybox(Shader *shader) {
  shader->use();
  shader->setFloat("skybox_multiplier", skybox_multiplier);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->material->textures[0].id);
  shader->setInt("skybox", 0);
  skybox->simple_draw();
}

int Scene::set_skybox_settings(std::string name, Shader *shader, int texture_unit) {
  glActiveTexture(GL_TEXTURE0+texture_unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->material->textures[0].id); // The skybox should only have 1 texture
  shader->setInt(name.c_str(), texture_unit);

  texture_unit++;
  return texture_unit;
}

void Scene::render_dirlights_shadow_map(Shader_Opacity_Triplet shaders) {
  for (auto dirlight : dirlights) {
    dirlight->bind_dirlight_framebuffer();
    dirlight->set_light_space(shaders.opaque);
    dirlight->set_light_space(shaders.full_transparency);
    dirlight->set_light_space(shaders.partial_transparency);
    draw_objects(shaders, Shader::DrawType::DEPTH_DIRLIGHT);
  }
}

int Scene::set_dirlight_settings(std::string name, Shader* shader, int texture_unit) {
  shader->setInt("nr_dirlights", dirlights.size());

  for (unsigned int i=0; i<dirlights.size(); i++) {
    dirlights[i]->set_object_settings(name+"["+std::to_string(i)+"]", shader);

    glActiveTexture(GL_TEXTURE0+texture_unit+i);
    glBindTexture(GL_TEXTURE_2D, dirlights[i]->depth_map);
    shader->setInt((name+"["+std::to_string(i)+"]"+".shadow_map").c_str(), texture_unit+i);
  }

  texture_unit += dirlights.size();
  return texture_unit;
}

void Scene::draw_dirlight(Shader *shader) {
  for (auto dirlight : dirlights) {
    dirlight->draw(shader);
  }
}

void Scene::render_pointlights_shadow_map(Shader_Opacity_Triplet shaders) {
  for (auto light : pointlights) {
    light->bind_pointlight_framebuffer();
    light->set_light_space(shaders.opaque);
    light->set_light_space(shaders.full_transparency);
    light->set_light_space(shaders.partial_transparency);
    draw_objects(shaders, Shader::DrawType::DEPTH_POINTLIGHT);
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

void Scene::draw_objects(Shader_Opacity_Triplet shaders, Shader::DrawType draw_type, int texture_unit, glm::vec3 camera_position) {
  std::vector<Transparent_Draw> partially_transparent_meshes;
  for (auto node : nodes) {
    node->draw(shaders, draw_type, &partially_transparent_meshes, glm::mat4(1.0f), texture_unit);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  std::sort(partially_transparent_meshes.begin(), partially_transparent_meshes.end(),
    [&camera_position](Transparent_Draw& obj1, Transparent_Draw& obj2){
      return glm::length2(camera_position-glm::vec3(obj1.model*glm::vec4(0.0f,0.0f,0.0f,1.0f))) >= glm::length2(camera_position-glm::vec3(obj2.model*glm::vec4(0.0f,0.0f,0.0f,1.0f)));
    }
  );

  // shaders.partial_transparency->use();
  for (auto draw_call : partially_transparent_meshes) {
    // shaders.partial_transparency->setMat4("model", draw_call.model);
    draw_call.mesh->draw(draw_call.shader, draw_type, draw_call.model, draw_call.texture_unit);
  }
  glBlendFunc(GL_ONE, GL_ZERO);
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
void Scene::add_node(std::shared_ptr<RootNode> node) {
  nodes.push_back(node);
}

void Scene::delete_node_at(unsigned int index) {
  Q_ASSERT_X(index < nodes.size(), "delete_node_at", "index is greater than vector nodes' size");
  nodes.erase(nodes.begin() + index);
}

void Scene::clear_nodes() {
  nodes.clear();
}

void Scene::add_dirlight(std::shared_ptr<DirectionalLight> dirlight) {
  dirlights.push_back(dirlight);
}

void Scene::delete_dirlight_at(unsigned int index) {
  Q_ASSERT_X(index < dirlights.size(), "delete_dirlight_at", "index is greater than vector dirlights' size");
  dirlights.erase(dirlights.begin() + index);
}

void Scene::clear_dirlights() {
  dirlights.clear();
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
