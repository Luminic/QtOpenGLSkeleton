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
  antialiasing_shader = nullptr;

  keys_pressed = nullptr;
  delta_time = nullptr;
  mouse_movement = nullptr;

  angle = 0.0f;
  fov = 45.0f;
}

void OpenGLWindow::set_inputs(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time) {
  this->keys_pressed = keys_pressed;
  this->delta_time = delta_time;
  this->mouse_movement = mouse_movement;
}

OpenGLWindow::~OpenGLWindow() {
  delete settings;
  delete camera;
  delete framebuffer_quad;
  delete object_shader;
  delete light_shader;
  delete sunlight_depth_shader;
  delete pointlight_depth_shader;
  delete skybox_shader;
  delete scene_shader;
  delete gaussian_blur_shader;
  delete post_processing_shader;
  delete antialiasing_shader;
}

void OpenGLWindow::initializeGL() {
  initializeOpenGLFunctions();

  #ifdef QT_DEBUG
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
  #endif

  qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

  settings = new Settings();

  load_shaders();

  scene = new Scene(this);
  settings->set_scene(scene);
  settings->set_sunlight(scene->sunlight);
  // settings->set_node(scene->floor, "Floor");
  // settings->set_node(scene->nanosuit, "Nanosuit");
  // settings->set_point_light(scene->light, "Pointlight");
  for (unsigned int i=0; i<scene->pointlights.size(); i++) {
    settings->set_point_light(scene->pointlights[i], ("Pointlight "+std::to_string(i)).c_str() );
  }

  framebuffer_quad = new Mesh();
  framebuffer_quad->initialize_plane(false);

  for (auto m : Scene::loaded_materials) {
    settings->set_material(m);
  }

  camera = new Camera();
  camera->exposure = 1.3f;
  camera->initialize_camera(keys_pressed, mouse_movement, delta_time);
  settings->set_camera(camera);

  create_framebuffer();
  create_scene_framebuffer();
  create_ping_pong_framebuffer();
  create_post_processing_framebuffer();

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
  antialiasing_shader = new Shader();

  sunlight_depth_shader->loadShaders("shaders/sunlight_depth_vertex.shader", "shaders/sunlight_depth_fragment.shader");
  pointlight_depth_shader->loadShaders("shaders/pointlight_depth_vertex.shader", "shaders/pointlight_depth_fragment.shader", "shaders/pointlight_depth_geometry.shader");

  object_shader->loadShaders("shaders/vertex.shader", "shaders/fragment.shader");//, "shaders/geometry.shader");
  light_shader->loadShaders("shaders/light_vertex.shader", "shaders/light_fragment.shader");
  skybox_shader->loadShaders("shaders/skybox_vertex.shader", "shaders/skybox_fragment.shader");
  scene_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/scene_fragment.shader");

  gaussian_blur_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/gaussian_blur_fragment.shader");
  post_processing_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/post_processing_fragment.shader");
  antialiasing_shader->loadShaders("shaders/framebuffer_vertex.shader", "shaders/antialiasing_fragment.shader");
}

void OpenGLWindow::create_framebuffer() {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  // Create the texture attachment
  int nr_color_buffers = sizeof(colorbuffers)/sizeof(colorbuffers[0]);
  scene->create_color_buffers(800, 600, nr_color_buffers, colorbuffers);

  // Create the renderbuffer
  glGenRenderbuffers(1, &renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  Q_ASSERT_X(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer creation", "incomplete framebuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::create_scene_framebuffer() {
  glGenFramebuffers(1, &scene_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer);
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  int nr_color_buffers = sizeof(scene_colorbuffers)/sizeof(scene_colorbuffers[0]);
  scene->create_color_buffers(800, 600, nr_color_buffers, scene_colorbuffers);

  Q_ASSERT_X(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer creation", "incomplete framebuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::create_ping_pong_framebuffer() {
  glGenFramebuffers(1, &ping_pong_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, ping_pong_framebuffer);

  // Note: these colorbuffers should be attached when they will be used
  // Their attachment locations here mean nothing
  scene->create_color_buffers(400, 300, 1, &bloom_colorbuffer);
  // The ping pong colorbuffers should be created last so the drawbuffers is 2
  scene->create_color_buffers(400, 300, 2, ping_pong_colorbuffers);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::create_post_processing_framebuffer() {
  glGenFramebuffers(1, &post_processing_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, post_processing_framebuffer);

  scene->create_color_buffers(800, 600, 1, &post_processing_colorbuffer);

  Q_ASSERT_X(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer creation", "incomplete framebuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLWindow::update_scene() {
  scene->update_scene();
  camera->update_cam();
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
  glm::mat4 sun_space;
  scene->sunlight->bind_sunlight_framebuffer(sunlight_depth_shader, sun_space);
  {
    int texture_unit = 0;
    scene->draw_objects(sunlight_depth_shader, false, 0);
  }

  // Draw the scene to the pointlight's depth buffer
  scene->render_lights_shadow_map(pointlight_depth_shader);

  // Draw the scene to our framebuffer
  //glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glViewport(0, 0, width(), height());
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glm::mat4 view = camera->view_matrix();

  glDepthMask(GL_FALSE);

  // Draw the skybox
  skybox_shader->use();
  skybox_shader->setMat4("view", glm::mat4(glm::mat3(view)));

  if (scene->display_type == POINTLIGHT_DEPTH) {
    skybox_shader->setInt("mode", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->pointlights[0]->depth_cubemap);
    skybox_shader->setInt("skybox", 0);
    scene->skybox->draw(skybox_shader);

  } else if (scene->display_type != SUNLIGHT_DEPTH) {
    skybox_shader->setInt("mode", 0);
    scene->draw_skybox(skybox_shader);

    // Draw the sun
    light_shader->use();
    light_shader->setMat4("view", glm::lookAt(glm::vec3(0.0f), camera->get_front(), camera->get_up()));
    scene->draw_sun(light_shader);

    glDepthMask(GL_TRUE);

    // Draw the light
    light_shader->use();
    light_shader->setMat4("view", view);
    scene->draw_light(light_shader);

    // Draw the objects
    object_shader->use();
    object_shader->setVec3("camera_position", camera->position);
    object_shader->setMat4("view", view);

    object_shader->setBool("use_volumetric_lighting", scene->use_volumetric_lighting);
    object_shader->setFloat("volumetric_multiplier", scene->volumetric_lighting_multiplier);
    object_shader->setFloat("volumetric_offset", scene->volumetric_lighting_offset);
    object_shader->setInt("steps", scene->volumetric_lighting_steps);
    object_shader->setFloat("henyey_greenstein_G_value", scene->henyey_greenstein_G_value);

    object_shader->setMat4("sun_space", sun_space);

    int texture_unit = 0;
    scene->set_skybox_settings("skybox", object_shader, texture_unit);
    scene->set_sunlight_settings("sunlight", object_shader, texture_unit);
    scene->set_light_settings("light", object_shader, texture_unit);
    scene->draw_objects(object_shader, true, texture_unit);
  }

  // Combine the scene into the scene framebuffer (so post-processing can be done on the entire scene)
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);

  scene_shader->use();
  scene_shader->setInt("display_type", scene->display_type);
  scene_shader->setFloat("exposure", camera->exposure);
  scene_shader->setFloat("bloom_threshold_upper", scene->bloom_threshold_upper);
  scene_shader->setFloat("bloom_threshold_lower", scene->bloom_threshold_lower);
  scene_shader->setInt("bloom_interpolation", scene->bloom_interpolation);

  switch (scene->display_type) {
    case SUNLIGHT_DEPTH:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene->sunlight->depth_map);
      scene_shader->setInt("screen_texture", 0);
      break;
    case POINTLIGHT_DEPTH:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      break;
    case VOLUMETRICS:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("screen_texture", 0);
      break;
    case BLOOM:
      scene_shader->setInt("display_type", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[1]);
      scene_shader->setInt("other_textures[0]", 1);
      break;
    case BRIGHT:
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ping_pong_colorbuffers[1], 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloom_colorbuffer, 0);
  glClearBufferfv(GL_COLOR, 1, glm::value_ptr(glm::vec4(0.0f,1.0f,0.0f,1.0f)));

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
  glBindFramebuffer(GL_FRAMEBUFFER, scene->antialiasing==FXAA ? post_processing_framebuffer : qt_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);

  post_processing_shader->use();

  switch (scene->display_type) {
    case SCENE:
      post_processing_shader->setInt("display_type", 0);
      post_processing_shader->setBool("gamma_correction", scene->antialiasing==NONE ? true : false);
      post_processing_shader->setFloat("bloom_multiplier", scene->bloom_multiplier);
      post_processing_shader->setFloat("bloom_offset", scene->bloom_offset);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      scene_shader->setInt("other_textures[0]", 1);
      break;
    case BLOOM:
      post_processing_shader->setInt("display_type", 1);
      post_processing_shader->setBool("gamma_correction", scene->antialiasing==NONE ? true : false);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      scene_shader->setInt("screen_texture", 0);
      break;
    case BRIGHT:
      post_processing_shader->setInt("display_type", 1);
      post_processing_shader->setBool("gamma_correction", scene->antialiasing==NONE ? true : false);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[1]);
      scene_shader->setInt("screen_texture", 0);
      break;
    default:
      post_processing_shader->setInt("display_type", 1);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      break;
  }

  framebuffer_quad->draw(post_processing_shader);

  if (scene->antialiasing == FXAA) {
    glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    antialiasing_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, post_processing_colorbuffer);
    antialiasing_shader->setInt("screen_texture", 0);

    framebuffer_quad->draw(antialiasing_shader);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  //glBindVertexArray(0);
}

void OpenGLWindow::update_perspective_matrix() {
  // Update perspective matrices
  projection = glm::perspective(glm::radians(fov), width()/float(height()), 0.1f, 100.0f);
  object_shader->use();
  object_shader->setMat4("projection", projection);
  light_shader->use();
  light_shader->setMat4("projection", projection);
  skybox_shader->use();
  skybox_shader->setMat4("projection", projection);
}

void OpenGLWindow::resizeGL(int w, int h) {
  // Update framebuffer textures
  int nr_color_buffers = sizeof(colorbuffers)/sizeof(colorbuffers[0]);
  scene->update_color_buffers_size(w, h, nr_color_buffers, colorbuffers);

  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

  // Update scene texture
  nr_color_buffers = sizeof(scene_colorbuffers)/sizeof(scene_colorbuffers[0]);
  scene->update_color_buffers_size(w, h, nr_color_buffers, scene_colorbuffers);

  // Update ping-pong textures
  scene->update_color_buffers_size(w/2, h/2, 2, ping_pong_colorbuffers);

  // Update bloom texture
  scene->update_color_buffers_size(w/2, h/2, 1, &bloom_colorbuffer);

  // Update post-processing texture
  scene->update_color_buffers_size(w, h, 1, &post_processing_colorbuffer);

  update_perspective_matrix();

  // glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

void OpenGLWindow::wheelEvent(QWheelEvent *event) {
  // qDebug() << "Pixel Delta:" << event->pixelDelta();
  // qDebug() << "Angle Delta:" << event->angleDelta();
  if (event->angleDelta().y() != 0) {
    event->accept();
    fov += event->angleDelta().y() / 60.0f;
    if (fov > 70.0f) fov = 70.0f;
    else if (fov < 5.0f) fov = 5.0f;
    update_perspective_matrix();
  } else {
    event->ignore();
  }
}
