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
  cube(nullptr),
  floor(nullptr),
  nanosuit(nullptr),
  light(nullptr),
  skybox(nullptr),
  framebuffer_quad(nullptr),
  object_shader(nullptr),
  light_shader(nullptr),
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
  delete cube;
  delete floor;
  delete nanosuit;
  delete light;
  delete skybox;
  delete framebuffer_quad;
  delete object_shader;
  delete light_shader;
  delete skybox_shader;
  delete framebuffer_shader;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  settings = new Settings();

  object_shader = new Shader();
  light_shader = new Shader();
  skybox_shader = new Shader();
  framebuffer_shader = new Shader();

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");
  skybox_shader->loadShaders("shaders/skybox_vertex.shader", "shaders/skybox_fragment.shader");
  framebuffer_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/framebuffer_fragment.shader");

  scene = new Scene(this);
  scene->camera->initialize_camera(keys_pressed, mouse_movement, delta_time);
  settings->set_camera(scene->camera);
  settings->set_point_light(scene->sunlight, "Sunlight");

  light = new PointLight(glm::vec3(1.2f, 0.6, 1.5f), glm::vec3(0.2f));
  settings->set_point_light(light, "Pointlight");

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
  settings->set_node(floor, "Floor");

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/sphere.fbx");
  //nanosuit = new Model("models/mouse/mouse.fbx");
  //nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->scale = glm::vec3(0.4f);
  settings->set_node(nanosuit, "Nanosuit");

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
  //glEnable(GL_CULL_FACE);
  //glEnable(GL_FRAMEBUFFER_SRGB);
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

  // Draw the scene to this framebuffer (instead of the screen)
  //glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glm::mat4 view = scene->camera->view_matrix();

  glDepthMask(GL_FALSE);

  // Draw the skybox (Where nothing else has been rendered)
  skybox_shader->use();
  skybox_shader->setMat4("view", glm::mat4(glm::mat3(view)));
  glActiveTexture(GL_TEXTURE0);
  skybox_shader->setInt("skybox", 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
  skybox->draw(skybox_shader);

  // Draw the sun
  scene->draw_sun(light_shader);

  glDepthMask(GL_TRUE);

  // Render the light
  glm::vec3 point_light_positions[] = {
  	glm::vec3( 0.7f,  0.2f, -2.0f),
  	glm::vec3( 2.3f, -3.3f,  4.0f),
  	glm::vec3(-4.0f,  2.0f,  12.0f),
  	glm::vec3( 0.0f,  0.0f,  3.0f)
  };

  light_shader->use();
  //light.shader->setMat4("model", light.get_model_matrix());
  light_shader->setMat4("view", view);

  for (int i=0; i<4; i++) {
    light->position = point_light_positions[i];
    light->draw(light_shader);
    light->set_object_settings(("light["+std::to_string(i)+"]").c_str(), object_shader);
  }

  // Render the cube
  object_shader->use();
  object_shader->setVec3("camera_position", scene->camera->position);
  object_shader->setMat4("view", view);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
  object_shader->setInt("skybox", 0);
  object_shader->setInt("test", 2);

  scene->set_sunlight_settings("sunlight", object_shader);

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
    object_shader->setMat4("model", model);
    cube->draw(object_shader, 1);
  }

  // Render the floor
  floor->draw(object_shader, glm::mat4(1.0f), 1);

  // Render the Nanosuit
  glm::mat4 model = nanosuit->get_model_matrix();
  nanosuit->draw(object_shader, model, 1);

  // Draw the framebuffer to the screen
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  framebuffer_shader->use();

  glActiveTexture(GL_TEXTURE0);
  framebuffer_shader->setInt("screen_texture", 0);
  glBindTexture(GL_TEXTURE_2D, texture_colorbuffer);

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
