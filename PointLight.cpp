#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 scale, glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic) :
  Light(position, scale, color, ambient, diffuse, specular),
  constant(constant),
  linear(linear),
  quadratic(quadratic)
{
  samples = 5;
  sample_radius = 0.02f;

  near_plane = 0.1f;
  far_plane = 45.0f;
}

PointLight::~PointLight() {
}

void PointLight::initialize_depth_framebuffer(unsigned int depth_map_width, unsigned int depth_map_height) {
  this->depth_map_width = depth_map_width;
  this->depth_map_height = depth_map_height;

  glGenTextures(1, &depth_cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
  for (int i=0; i<6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &depth_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointLight::bind_pointlight_framebuffer(Shader *depth_shader) {
  glViewport(0, 0, depth_map_width, depth_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, depth_framebuffer);

  glClear(GL_DEPTH_BUFFER_BIT);

  depth_shader->use();
  
  glm::mat4 pointlight_projection = glm::perspective(glm::radians(90.0f), float(depth_map_width)/depth_map_height, near_plane, far_plane);

  glm::mat4 pointlight_views[6] = {
    pointlight_views[0] = glm::lookAt(position, position+glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),
    pointlight_views[1] = glm::lookAt(position, position+glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),
    pointlight_views[2] = glm::lookAt(position, position+glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f)),
    pointlight_views[3] = glm::lookAt(position, position+glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f)),
    pointlight_views[4] = glm::lookAt(position, position+glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),
    pointlight_views[5] = glm::lookAt(position, position+glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f,-1.0f, 0.0f))
  };

  for (int i=0; i<6; i++) {
    depth_shader->setMat4(("light_spaces["+std::to_string(i)+"]").c_str(), pointlight_projection*pointlight_views[i]);
  }
  depth_shader->setVec3("pointlight_position", position);
  depth_shader->setFloat("far_plane", far_plane);
}

void PointLight::set_object_settings(std::string name, Shader *shader) {
  Light::set_object_settings(name, shader);

  shader->setVec3((name+".position").c_str(), position);

  shader->setFloat((name+".constant").c_str(), constant);
  shader->setFloat((name+".linear").c_str(), linear);
  shader->setFloat((name+".quadratic").c_str(), quadratic);

  shader->setInt((name+".samples").c_str(), samples);
  shader->setFloat((name+".sample_radius").c_str(), sample_radius);
}
