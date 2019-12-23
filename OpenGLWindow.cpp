#include <string>

#include <QDebug>
#include <QString>
#include <QFile>
#include <QImage>
#include <QMatrix4x4>

#include "OpenGLWindow.h"

OpenGLWindow::OpenGLWindow(QWidget *parent) : QOpenGLWidget(parent),
  // Declare all pointers as null just in case
  settings(nullptr),
  scene(nullptr),
  framebuffer_quad(nullptr),
  object_shader(nullptr),
  light_shader(nullptr),
  depth_shader(nullptr),
  skybox_shader(nullptr),
  framebuffer_shader(nullptr),
  keys_pressed(nullptr),
  delta_time(nullptr),
  mouse_movement(nullptr)
{
  angle = 0.0f;
}

void OpenGLWindow::set_inputs(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time) {
  this->keys_pressed = keys_pressed;
  this->delta_time = delta_time;
  this->mouse_movement = mouse_movement;
}

OpenGLWindow::~OpenGLWindow() {
  delete settings;
  delete framebuffer_quad;
  delete object_shader;
  delete light_shader;
  delete depth_shader;
  delete skybox_shader;
  delete framebuffer_shader;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  settings = new Settings();

  object_shader = new Shader();
  light_shader = new Shader();
  depth_shader = new Shader();
  skybox_shader = new Shader();
  framebuffer_shader = new Shader();

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");
  depth_shader->loadShaders("shaders/depth_vertex.shader", "shaders/depth_fragment.shader");
  skybox_shader->loadShaders("shaders/skybox_vertex.shader", "shaders/skybox_fragment.shader");
  framebuffer_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/framebuffer_fragment.shader");

  scene = new Scene(this);
  scene->camera->initialize_camera(keys_pressed, mouse_movement, delta_time);
  settings->set_scene(scene);
  settings->set_camera(scene->camera);
  settings->set_sunlight(scene->sunlight);
  settings->set_node(scene->floor, "Floor");
  settings->set_node(scene->nanosuit, "Nanosuit");
  settings->set_point_light(scene->light, "Pointlight");

  framebuffer_quad = new Mesh();
  framebuffer_quad->initialize_plane(false);

  for (auto m : Scene::loaded_materials) {
    settings->set_material(m);
  }

  // Create the framebuffer object
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // Create the texture attachment (for the framebuffer)
  glGenTextures(1, &texture_colorbuffer);
  glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_colorbuffer, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  // Create the renderbuffer (for the framebuffer)
  glGenRenderbuffers(1, &renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  // Check if the framebuffer is complete
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qDebug() << "INCOMPLETE FRAMEBUFFER!\n";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Not really needed

  //glEnable(GL_FRAMEBUFFER_SRGB);
  //glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glClearColor(0.1, 0.1, 0.2, 1.0);
}

void OpenGLWindow::update_scene() {
  scene->update_scene();
  angle++;
  if (angle>=360) angle=0;
  update();
}

void OpenGLWindow::paintGL() {
  // Note: never call this function directly--call update() instead.

  // Get QT's default framebuffer binding
  int qt_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &qt_framebuffer);

  glEnable(GL_DEPTH_TEST);

  // Draw the scene to the sunlight's depth buffer to create the sunlight's depth map
  glViewport(0, 0, scene->sunlight->depth_map_width, scene->sunlight->depth_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, scene->sunlight->depth_framebuffer);

  glClear(GL_DEPTH_BUFFER_BIT);
  //glCullFace(GL_FRONT);

  depth_shader->use();
  glm::mat4 sunlight_projection = glm::ortho(
    -scene->sunlight->x_view_size/2, scene->sunlight->x_view_size/2,
    -scene->sunlight->y_view_size/2, scene->sunlight->y_view_size/2,
    scene->sunlight->near_plane, scene->sunlight->far_plane
  );
  glm::mat4 sunlight_view = glm::lookAt(
    scene->sunlight->get_position()*2.0f,
    glm::vec3(0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
  );

  depth_shader->setMat4("light_space", sunlight_projection*sunlight_view);

  scene->draw_objects(depth_shader, false, 0);


  // Draw the scene to our framebuffer
  //glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glViewport(0, 0, width(), height());
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glm::mat4 view = scene->camera->view_matrix();

  glDepthMask(GL_FALSE);

  // Draw the skybox
  skybox_shader->use();
  skybox_shader->setMat4("view", glm::mat4(glm::mat3(view)));
  scene->draw_skybox(skybox_shader);

  // Draw the sun
  scene->draw_sun(light_shader);

  glDepthMask(GL_TRUE);

  // Draw the light
  light_shader->use();
  light_shader->setMat4("view", view);
  scene->draw_light(light_shader);

  // Draw the objects
  object_shader->use();
  object_shader->setVec3("camera_position", scene->camera->position);
  object_shader->setMat4("view", view);

  object_shader->setBool("use_volumetric_lighting", scene->use_volumetric_lighting);
  object_shader->setFloat("volumetric_multiplier", scene->volumetric_lighting_multiplier);
  object_shader->setInt("steps", scene->volumetric_lighting_steps);
  object_shader->setFloat("henyey_greenstein_G_value", scene->henyey_greenstein_G_value);

  object_shader->setMat4("light_space", sunlight_projection*sunlight_view);

  scene->set_skybox_settings("skybox", object_shader, 0);
  scene->set_sunlight_settings("sunlight", object_shader, 1);
  scene->set_light_settings("light", object_shader);

  scene->draw_objects(object_shader, true, 2);

  // Draw the framebuffer to the screen
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  framebuffer_shader->use();
  framebuffer_shader->setInt("display_type", scene->display_type);

  glActiveTexture(GL_TEXTURE0);
  switch (scene->display_type) {
    case 1:
    glBindTexture(GL_TEXTURE_2D, scene->sunlight->depth_map);
    break;
    default:
    glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);
    break;
  }
  framebuffer_shader->setInt("screen_texture", 0);

  framebuffer_quad->draw(framebuffer_shader);

  //glBindVertexArray(0);
}

void OpenGLWindow::resizeGL(int w, int h) {
  // Update framebuffer textures
  glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  // Update perspective matrices
  projection = glm::perspective(glm::radians(45.0f), width()/float(height()), 0.1f, 100.0f);
  object_shader->use();
  object_shader->setMat4("projection", projection);
  light_shader->use();
  light_shader->setMat4("projection", projection);
  skybox_shader->use();
  skybox_shader->setMat4("projection", projection);
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
