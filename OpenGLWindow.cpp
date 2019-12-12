#include <string>

#include <QDebug>
#include <QString>
#include <QFile>
#include <QImage>
#include <QMatrix4x4>

#include "OpenGLWindow.h"

OpenGLWindow::OpenGLWindow(QWidget *parent) :
  QOpenGLWidget(parent),
  settings(new Settings(this)),
  scene(new Scene()),
  object({glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), 64.0f}),
  light(glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, 1.0f, 1.0f, glm::vec3(1.2f, 0.6, 1.5f), glm::vec3(0.2f)),
  object_shader(new Shader()),
  light_shader(new Shader())
{
  angle = 0.0f;
  light.set_falloff(1.0f, 0.09f, 0.032f);
  settings->set_camera(scene->camera);
  settings->set_scene(scene);
  settings->set_point_light(&scene->sunlight, "Sunlight");
  settings->set_object(&object, "Box");
  settings->set_point_light(&light, "Pointlights");
}

OpenGLWindow::~OpenGLWindow() {
  delete settings;
  delete scene;
  delete object_shader;
  delete light_shader;
  delete nanosuit;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");

  object.initialize_shader(object_shader);
  object.initialize_buffers();
  object.load_texture("textures/container2.png", "diffuse1");
  object.load_texture("textures/container2_specular.png", "specular1");

  light.initialize_shader(light_shader);
  light.initialize_buffers();

  scene->initialize_scene(light_shader, object.get_VBO(), object.get_EBO());

  nanosuit = new Model("models/mouse/mouse.fbx");
  //nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->set_scale(glm::vec3(0.5f));
  settings->set_object(nanosuit, "Nanosuit");

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Not really needed
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
    light_shader->use();
    light.set_position(point_light_positions[i]);
    light_shader->setMat4("model", light.get_model_matrix());
    light.draw();
    light.set_object_settings(("light["+std::to_string(i)+"]").c_str(), object_shader);
  }

  // Render the cube
  object_shader->use();
  object_shader->setVec3("camera_position", scene->camera->get_position());
  object_shader->setMat4("view", view);

  scene->set_sunlight_settings("sunlight", object_shader);
  object_shader->setInt("number_diffuse_textures", 1);
  object_shader->setInt("number_specular_textures", 1);

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
    model = glm::scale(model, object.get_scale());
    object_shader->setMat4("model", model);
    object.draw();
  }

  // Render the Nanosuit

  object_shader->use();
  glm::mat4 model = nanosuit->get_model_matrix();
  //object_shader->setMat4("model", model);

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
