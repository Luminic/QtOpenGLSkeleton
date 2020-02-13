#include <QDebug>

#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular) :
  Light(position, scale, color, ambient, diffuse, specular)
{
  Mesh* dirlight_mesh = new Mesh();
  dirlight_mesh->initialize_plane();
  meshes.push_back(std::shared_ptr<Mesh>(dirlight_mesh));

  dirlight_space = glm::mat4(1.0f);
  x_view_size = 5.0f;
  y_view_size = 5.0f;
  near_plane = 0.1f;
  far_plane = 45.0f;
}

DirectionalLight::~DirectionalLight() {}

void DirectionalLight::set_object_settings(std::string name, Shader *shader) {
  Light::set_object_settings(name, shader);

  shader->setVec3((name+".direction").c_str(), -direction);
  shader->setMat4((name+".light_space").c_str(), dirlight_space);
}

void DirectionalLight::initialize_depth_framebuffer(unsigned int depth_map_width, unsigned int depth_map_height) {
  this->depth_map_width = depth_map_width;
  this->depth_map_height = depth_map_height;

  glGenFramebuffers(1, &depth_framebuffer);

  glGenTextures(1, &depth_map);
  glBindTexture(GL_TEXTURE_2D, depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float border_color[] = {0.0f,0.0f,0.0f,1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qDebug() << "INCOMPLETE FRAMEBUFFER!\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirectionalLight::bind_dirlight_framebuffer(Shader* depth_shader) {
  glViewport(0, 0, depth_map_width, depth_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);

  glClear(GL_DEPTH_BUFFER_BIT);

  glm::mat4 sunlight_projection = glm::ortho(
    -x_view_size/2, x_view_size/2,
    -y_view_size/2, y_view_size/2,
    near_plane, far_plane
  );
  glm::mat4 sunlight_view = glm::lookAt(
    position,
    position+direction,
    glm::vec3(0.0f, 1.0f, 0.0f)
  );
  dirlight_space = sunlight_projection*sunlight_view;

  depth_shader->use();
  depth_shader->setMat4("light_space", dirlight_space);
}

glm::mat4 DirectionalLight::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  if (direction.z == 0) direction.z = 0.000001;
  model = glm::rotate(model, glm::atan(direction.x,direction.z), glm::vec3(0.0f,1.0f,0.0f));
  model = glm::rotate(model, glm::acos(glm::normalize(direction).y), glm::vec3(1.0f,0.0f,0.0f));
  model = glm::scale(model, glm::vec3(x_view_size/2, 1.0f, y_view_size/2));
  return model;
}

glm::vec3 DirectionalLight::get_direction() {
  return direction;
}

void DirectionalLight::set_direction(float yaw, float pitch) {
  direction = glm::vec3(
    glm::sin(yaw)*glm::cos(pitch),
    glm::sin(yaw)*glm::sin(pitch),
    glm::cos(yaw)*glm::cos(pitch)
  );
}

void DirectionalLight::set_direction(glm::vec3 dir) {
  direction = dir;
}

void DirectionalLight::set_scale(glm::vec3 sca) {
  #ifdef QT_DEBUG
    qDebug() << "Setting scale is disabled for directional lights; use x_view_size and y_view_size instead";
  #endif
}

void DirectionalLight::set_rotation(glm::vec3 rot) {
  #ifdef QT_DEBUG
    qDebug() << "Setting rotation is disabled for directional lights; use direction instead";
  #endif
}
