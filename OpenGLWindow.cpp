#include <string>
#include <vector>

#include <QDebug>
#include <QString>
#include <QFile>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>

#include "OpenGLWindow.h"

OpenGLWindow::OpenGLWindow(QWidget *parent) : QOpenGLWidget(parent) {
  // Declare all pointers as null just in case
  settings = nullptr;
  scene = nullptr;
  framebuffer_quad = nullptr;

  sunlight_depth_shader = nullptr;
  pointlight_depth_shader = nullptr;
  object_shader = nullptr;
  light_shader = nullptr;
  skybox_shader = nullptr;
  scene_shader = nullptr;
  gaussian_blur_shader = nullptr;
  post_processing_shader = nullptr;

  keys_pressed = nullptr;
  delta_time = nullptr;
  mouse_movement = nullptr;

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
  delete sunlight_depth_shader;
  delete pointlight_depth_shader;
  delete skybox_shader;
  delete scene_shader;
  delete gaussian_blur_shader;
  delete post_processing_shader;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  QOpenGLContext *ctx = QOpenGLContext::currentContext();
  QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
  if (!logger->initialize()) {
    qDebug() << "QOpenGLDebugLogger failed to initialize.";
  }
  if (!ctx->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
    qDebug() << "KHR Debug extension unavailable.";
  }

  connect(logger, &QOpenGLDebugLogger::messageLogged, this,
    [](const QOpenGLDebugMessage &message){
      if (message.severity() != QOpenGLDebugMessage::NotificationSeverity) {
        qDebug() << message;

      }
    }
  );
  logger->startLogging();

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  settings = new Settings();

  load_shaders();

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

  create_framebuffer();
  create_scene_framebuffer();
  create_ping_pong_framebuffer();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Not really needed

  //glEnable(GL_FRAMEBUFFER_SRGB);
  //glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_DEBUG_OUTPUT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

void OpenGLWindow::load_shaders() {
  sunlight_depth_shader = new Shader();
  pointlight_depth_shader = new Shader();
  object_shader = new Shader();
  light_shader = new Shader();
  skybox_shader = new Shader();
  scene_shader = new Shader();
  gaussian_blur_shader = new Shader();
  post_processing_shader = new Shader();

  sunlight_depth_shader->loadShaders("shaders/sunlight_depth_vertex.shader", "shaders/sunlight_depth_fragment.shader");
  pointlight_depth_shader->loadShaders("shaders/pointlight_depth_vertex.shader", "shaders/pointlight_depth_fragment.shader", "shaders/pointlight_depth_geometry.shader");

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");//, "shaders/geometry.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");
  skybox_shader->loadShaders("shaders/skybox_vertex.shader", "shaders/skybox_fragment.shader");
  scene_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/scene_fragment.shader");

  gaussian_blur_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/gaussian_blur_fragment.shader");
  post_processing_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/framebuffer_fragment.shader");
}

void OpenGLWindow::create_framebuffer() {
  // Create the framebuffer object
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // Create the texture attachment (for the framebuffer)
  int size = sizeof(colorbuffers)/sizeof(colorbuffers[0]);
  glGenTextures(size, colorbuffers);
  for (int i=0; i<size; i++) {
    glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, colorbuffers[i], 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);
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
}

void OpenGLWindow::create_scene_framebuffer() {
  // Create the scene framebuffer
  glGenFramebuffers(1, &scene_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer);
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  int size = sizeof(scene_colorbuffers)/sizeof(scene_colorbuffers[0]);
  glGenTextures(size, scene_colorbuffers);
  for (int i=0; i<size; i++) {
    glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, scene_colorbuffers[i], 0);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qDebug() << "INCOMPLETE FRAMEBUFFER!\n";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::create_ping_pong_framebuffer() {
  // Create the ping-pong framebuffer
  glGenFramebuffers(1, &ping_pong_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  glGenTextures(2, ping_pong_colorbuffers);
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);
  for (int i=0; i<2; i++) {
    glBindTexture(GL_TEXTURE_2D, ping_pong_colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 400, 300, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Notice how the colorbuffers aren't attached
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  create_gaussian_blur_colorbuffer();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::create_gaussian_blur_colorbuffer() {
  // Create the Gaussian Blur colorbuffer (and attach to the ping-pong framebuffers)
  // This blur will add all the results of the ping-pong colorbuffers together
  glGenTextures(1, &bloom_colorbuffer);
  glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 400, 300, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloom_colorbuffer, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
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

  // Get QT's default framebuffer binding
  int qt_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &qt_framebuffer); // QOpenGLWidget::defaultFramebufferObject() also works

  glEnable(GL_DEPTH_TEST);

  // Draw the scene to the sunlight's depth buffer to create the sunlight's depth map
  glViewport(0, 0, scene->sunlight->depth_map_width, scene->sunlight->depth_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, scene->sunlight->depth_framebuffer);

  glClear(GL_DEPTH_BUFFER_BIT);

  sunlight_depth_shader->use();
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

  sunlight_depth_shader->setMat4("light_space", sunlight_projection*sunlight_view);

  scene->draw_objects(sunlight_depth_shader, false, 0);

  // Draw the scene to the pointlight's depth buffer
  glViewport(0, 0, scene->light->depth_map_width, scene->light->depth_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, scene->light->depth_framebuffer);

  glClear(GL_DEPTH_BUFFER_BIT);

  pointlight_depth_shader->use();
  glm::mat4 pointlight_projection = glm::perspective(glm::radians(90.0f), float(scene->light->depth_map_width)/scene->light->depth_map_height, scene->light->near_plane, scene->light->far_plane);
  std::vector<glm::mat4> pointlight_views;
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)));
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)));
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f)));
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f)));
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3( 0.0f,-1.0f, 0.0f)));
  pointlight_views.push_back(glm::lookAt(scene->light->position, scene->light->position+glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f,-1.0f, 0.0f)));

  for (int i=0; i<6; i++) {
    pointlight_depth_shader->setMat4(("light_spaces["+std::to_string(i)+"]").c_str(), pointlight_projection*pointlight_views[i]);
  }
  pointlight_depth_shader->setVec3("pointlight_position", scene->light->position);
  pointlight_depth_shader->setFloat("far_plane", scene->light->far_plane);

  scene->draw_objects(pointlight_depth_shader, false, 0);

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

  if (scene->display_type == 2) {
    skybox_shader->setInt("mode", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->light->depth_cubemap);
    skybox_shader->setInt("skybox", 0);
    scene->skybox->draw(skybox_shader);

  } else if (scene->display_type != 1) {
    skybox_shader->setInt("mode", 0);
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
    object_shader->setFloat("volumetric_offset", scene->volumetric_lighting_offset);
    object_shader->setInt("steps", scene->volumetric_lighting_steps);
    object_shader->setFloat("henyey_greenstein_G_value", scene->henyey_greenstein_G_value);

    object_shader->setMat4("light_space", sunlight_projection*sunlight_view);

    scene->set_skybox_settings("skybox", object_shader, 0);
    scene->set_sunlight_settings("sunlight", object_shader, 1);
    scene->set_light_settings("light", object_shader, 2);

    scene->draw_objects(object_shader, true, 3);
  }

  // Combine the scene into the scene framebuffer (so post-processing can be done on the entire scene)
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);

  scene_shader->use();
  scene_shader->setInt("display_type", scene->display_type);
  scene_shader->setFloat("exposure", scene->camera->exposure);
  scene_shader->setFloat("bloom_threshold_upper", scene->bloom_threshold_upper);
  scene_shader->setFloat("bloom_threshold_lower", scene->bloom_threshold_lower);
  scene_shader->setInt("bloom_interpolation", scene->bloom_interpolation);

  switch (scene->display_type) {
    case 1:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene->sunlight->depth_map);
      scene_shader->setInt("screen_texture", 0);
      break;
    case 2:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      break;
    case 3:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("screen_texture", 0);
      break;
    case 4:
      scene_shader->setInt("display_type", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("other_textures[0]", 1);
      break;
    case 5:
      scene_shader->setInt("display_type", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("other_textures[0]", 1);
      break;
    default:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("other_textures[0]", 1);
      break;
  }

  framebuffer_quad->draw(scene_shader);

  // Create bloom effect with gaussian blur
  glViewport(0, 0, width()/2, height()/2);
  // Clear the buffers (if they aren't cleared it causes problems when window is resized)
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[0], 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[1], 0);
  glClearBufferfv(GL_COLOR, 0, glm::value_ptr(glm::vec4(0.0f,1.0f,0.0f,1.0f)));

  gaussian_blur_shader->use();

  bool horizontal=true;
  for (int i=0; i<scene->bloom_applications; i++) {
    gaussian_blur_shader->setBool("horizontal", horizontal);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_pong_colorbuffers[i%2], 0);

    glActiveTexture(GL_TEXTURE0);
    if (i==0) {
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[1]);
      glBlendFunci(1, GL_ONE, GL_ZERO);
    } else {
      glBindTexture(GL_TEXTURE_2D, ping_pong_colorbuffers[(i+1)%2]);
      glBlendFunci(1, GL_ONE, GL_ONE); // Add the individual blurs together
    }
    gaussian_blur_shader->setInt("image", 0);

    framebuffer_quad->draw(gaussian_blur_shader);

    horizontal = !horizontal;
  }

  glBlendFunc(GL_ONE, GL_ZERO);

  glViewport(0, 0, width(), height());
  glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  post_processing_shader->use();
  post_processing_shader->setInt("display_type", scene->display_type);

  switch (scene->display_type) {
    case 0:
      post_processing_shader->setFloat("bloom_multiplier", scene->bloom_multiplier);
      post_processing_shader->setFloat("bloom_offset", scene->bloom_offset);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      scene_shader->setInt("other_textures[0]", 1);
      break;
    case 4:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      scene_shader->setInt("other_textures[0]", 0);
      break;
    case 5:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[1]);
      scene_shader->setInt("other_textures[0]", 0);
      break;
    default:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      break;
  }

  framebuffer_quad->draw(post_processing_shader);

  //glBindVertexArray(0);
}

void OpenGLWindow::resizeGL(int w, int h) {
  // Update framebuffer textures
  int size = sizeof(colorbuffers)/sizeof(colorbuffers[0]);
  for (int i=0; i<size; i++) {
    glBindTexture(GL_TEXTURE_2D, colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  // Update scene texture
  size = sizeof(scene_colorbuffers)/sizeof(scene_colorbuffers[0]);
  for (int i=0; i<size; i++) {
    glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  // Update ping-pong textures
  for (int i=0; i<2; i++) {
    glBindTexture(GL_TEXTURE_2D, ping_pong_colorbuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, NULL);
  }
  // Update bloom texture
  glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w/2, h/2, 0, GL_RGBA, GL_FLOAT, NULL);
  // Update perspective matrices
  projection = glm::perspective(glm::radians(45.0f), width()/float(height()), 0.1f, 100.0f);
  object_shader->use();
  object_shader->setMat4("projection", projection);
  light_shader->use();
  light_shader->setMat4("projection", projection);
  skybox_shader->use();
  skybox_shader->setMat4("projection", projection);
  // glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}
