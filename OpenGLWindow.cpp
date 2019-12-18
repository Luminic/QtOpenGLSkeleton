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
  light(nullptr),
  nanosuit(nullptr),
  object_shader(nullptr),
  light_shader(nullptr),
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
  delete light;
  delete nanosuit;
  delete object_shader;
  delete light_shader;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  settings = new Settings();

  scene = new Scene(this);
  scene->camera->initialize_camera(keys_pressed, mouse_movement, delta_time);
  settings->set_camera(scene->camera);
  settings->set_point_light(scene->sunlight, "Sunlight");

  cube = new Mesh();

  light = new PointLight(glm::vec3(1.2f, 0.6, 1.5f), glm::vec3(0.2f));
  settings->set_point_light(light, "Pointlight");

  object_shader = new Shader();
  light_shader = new Shader();

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");

  cube->initialize_cube();
  cube->material = new Material();
  cube->material->load_texture("textures/container2.png", ALBEDO_MAP);
  cube->material->load_texture("textures/container2_specular.png", SPECULAR_MAP);
  cube->material->metalness = 0.0f;
  cube->material = Scene::is_material_loaded(cube->material);

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/material_test.fbx");
  //nanosuit = new Model("models/mouse/mouse.fbx");
  //nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->scale = glm::vec3(0.2f);
  settings->set_node(nanosuit, "Nanosuit");

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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glm::mat4 view = scene->camera->view_matrix();

  scene->draw_sun(light_shader);

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
    cube->draw(object_shader);
  }

  // Render the Nanosuit

  //object_shader->use();
  glm::mat4 model = nanosuit->get_model_matrix();
  //object_shader->setMat4("model", model);
  //model = glm::mat4(1.0f);
  nanosuit->draw(object_shader, model);

  //glBindVertexArray(0);
}

void OpenGLWindow::resizeGL(int w, int h) {
  projection = glm::perspective(glm::radians(45.0f), width()/float(height()), 0.1f, 100.0f);
  object_shader->use();
  object_shader->setMat4("projection", projection);
  light_shader->use();
  light_shader->setMat4("projection", projection);
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
