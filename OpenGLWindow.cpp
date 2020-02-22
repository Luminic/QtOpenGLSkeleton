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

  delete dirlight_depth_opaque_shader;
  delete dirlight_depth_full_transparency_shader;
  delete dirlight_depth_partial_transparency_shader;

  delete pointlight_depth_opaque_shader;
  delete pointlight_depth_full_transparency_shader;
  delete pointlight_depth_partial_transparency_shader;

  delete object_opaque_shader;
  delete object_full_transparency_shader;
  delete object_partial_transparency_shader;

  delete light_shader;
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

  camera = new Camera();
  camera->exposure = 1.6f;
  camera->initialize_camera(keys_pressed, mouse_movement, delta_time);
  settings->set_camera(camera);

  scene = new Scene(this);
  settings->set_scene(scene);

  DirectionalLight* dirlight = new DirectionalLight(glm::vec3(-6.0f, 7.0f, -10.0f), glm::vec3(0.2f));
  dirlight->set_direction(glm::vec3(2.0f,-3.0f,4.0f));
  dirlight->x_view_size = 35;
  dirlight->y_view_size = 35;
  dirlight->initialize_depth_framebuffer(2048,2048);
  dirlight->ambient = 0.5f;
  dirlight->diffuse = 2.5f;
  dirlight->specular = 2.5f;
  dirlight->set_visibility(false);
  scene->add_dirlight(std::shared_ptr<DirectionalLight>(dirlight));
  settings->set_dirlight(dirlight);

  dirlight = new DirectionalLight(glm::vec3(3.6f, 4.6f, -2.7f), glm::vec3(0.2f));
  dirlight->set_direction(glm::vec3(-1.0f,-2.0f,1.0f));
  dirlight->initialize_depth_framebuffer(2048,2048);
  dirlight->ambient = 0.5f;
  dirlight->diffuse = 2.5f;
  dirlight->specular = 2.5f;
  scene->add_dirlight(std::shared_ptr<DirectionalLight>(dirlight));
  settings->set_dirlight(dirlight);

  // settings->set_node(scene->floor, "Floor");
  // settings->set_node(scene->nanosuit, "Nanosuit");
  // settings->set_point_light(scene->light, "Pointlight");

  glm::vec3 light_positions[2] = {
    glm::vec3( 2.4f, 1.9, 2.2f),
    glm::vec3(-5.0f, 2.2, 2.0f)
  };

  for (int i=0; i<2; i++) {
    PointLight* light = new PointLight(light_positions[i], glm::vec3(0.2f));
    light->initialize_depth_framebuffer(1024,1024);
    light->color = glm::vec3(1.5);
    scene->add_pointlight(std::shared_ptr<PointLight>(light));
  }

  for (unsigned int i=0; i<scene->get_pointlights().size(); i++) {
    settings->set_point_light(scene->get_pointlights()[i].get(), ("Pointlight "+std::to_string(i)).c_str() );
  }

  //nanosuit = new Model("models/parenting_test/parenting_test.fbx");
  // nanosuit = new Model("models/raygun/raygun.fbx");
  //nanosuit = new Model("models/material_test/sphere.fbx");
  // Model* nanosuit = new Model("models/lightray_test/wall2.fbx");
  Model* nanosuit = new Model("models/nanosuit/nanosuit.obj");
  nanosuit->set_scale(glm::vec3(0.3f));
  nanosuit->set_rotation(glm::vec3(180.0f,0.0f,0.0f));
  nanosuit->set_position(glm::vec3(0.0f,-3.5f,0.0f));
  scene->add_node(std::shared_ptr<Node>(nanosuit));

  std::shared_ptr<Mesh> cube = std::make_shared<Mesh>();
  cube->initialize_cube();
  cube->material = new Material();
  cube->material->load_texture("textures/container2.png", ALBEDO_MAP);
  cube->material->load_texture("textures/container2_specular.png", METALNESS_MAP);
  cube->material->load_texture("textures/container2_specular.png", ROUGHNESS_MAP);
  cube->material->metalness = 1.0f;
  cube->material = Scene::is_material_loaded(cube->material);

  glm::vec3 cube_positions[10] = {
    glm::vec3( 0.0f,  0.0f,  2.0f),
    glm::vec3( 2.0f,  5.0f,  17.0f),
    glm::vec3(-1.5f, -2.2f,  4.5f),
    glm::vec3(-3.8f, -2.0f,  14.3f),
    glm::vec3( 2.4f, -0.4f,  5.5f),
    glm::vec3(-1.7f,  3.0f,  9.5f),
    glm::vec3( 1.3f, -2.0f,  4.5f),
    glm::vec3( 1.5f,  2.0f,  4.5f),
    glm::vec3( 1.5f,  0.2f,  3.5f),
    glm::vec3(-1.3f,  1.0f,  3.5f)
  };

  for (int i=0; i<10; i++) {
    Node* n = new Node(glm::mat4(1.0f), cube_positions[i], glm::vec3(1.0f), glm::vec3(3.2f*i,4.6f*i,-7.0f*i));
    n->add_mesh(cube);
    scene->add_node(std::shared_ptr<Node>(n));
  }

  std::shared_ptr<Mesh> grass = std::make_shared<Mesh>();
  grass->initialize_plane(false);
  grass->material = new Material();
  grass->material->opacity_map = grass->material->load_texture("textures/grass.png", ALBEDO_MAP, ImageLoading::Options::TRANSPARENCY | ImageLoading::Options::FLIP_ON_LOAD | ImageLoading::Options::CLAMPED | ImageLoading::Options::ADD_TO_MATERIAL);
  grass->material->opacity_map.type = OPACITY_MAP;
  grass->set_transparency(FULL_TRANSPARENCY);
  grass->material = Scene::is_material_loaded(grass->material);

  glm::vec3 grass_positions[5] = {
    glm::vec3( 0.0f, -3.0f,  1.0f),
    glm::vec3( 1.0f, -3.0f,  7.0f),
    glm::vec3(-2.5f, -3.0f,  3.5f),
    glm::vec3(-2.8f, -3.0f,  6.0f),
    glm::vec3( 2.4f, -3.0f,  2.5f)
  };

  for (int i=0; i<10; i++) {
    Node* n = new Node(glm::mat4(1.0f), grass_positions[i/2], glm::vec3(0.5f), glm::vec3(90.0f*(i%2),0.0f,0.0f));
    n->add_mesh(grass);
    scene->add_node(std::shared_ptr<Node>(n));
  }

  Node* floor = new Node(glm::mat4(1.0f), glm::vec3(0.0f,-3.5f,4.5f), glm::vec3(7.0f,1.0f,7.0f));
  Mesh* floor_mesh = new Mesh();
  floor_mesh->initialize_plane(true, 3.0f);
  floor_mesh->material = new Material();
  floor_mesh->material->load_texture("textures/wood_floor.png", ALBEDO_MAP);
  floor_mesh->material->ambient = 0.2f;
  floor_mesh->material->diffuse = 0.6f;
  floor_mesh->material->specular = 0.3f;
  floor_mesh->material->roughness = 0.66f;
  floor_mesh->material = Scene::is_material_loaded(floor_mesh->material);

  floor->add_mesh(std::shared_ptr<Mesh>(floor_mesh));
  floor->set_scale(glm::vec3(14.0f,1.0f,7.0f));
  scene->add_node(std::shared_ptr<Node>(floor));


  framebuffer_quad = new Mesh();
  framebuffer_quad->initialize_plane(false);

  for (auto m : Scene::loaded_materials) {
    settings->set_material(m);
    qDebug() << m->textures[0].path.c_str();
  }

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
  dirlight_depth_opaque_shader = new Shader();
  dirlight_depth_full_transparency_shader = new Shader();
  dirlight_depth_partial_transparency_shader = new Shader();

  pointlight_depth_opaque_shader = new Shader();
  pointlight_depth_full_transparency_shader = new Shader();
  pointlight_depth_partial_transparency_shader = new Shader();

  object_opaque_shader = new Shader();
  object_full_transparency_shader = new Shader();
  object_partial_transparency_shader = new Shader();

  light_shader = new Shader();
  skybox_shader = new Shader();
  scene_shader = new Shader();

  gaussian_blur_shader = new Shader();
  post_processing_shader = new Shader();
  antialiasing_shader = new Shader();

  dirlight_depth_opaque_shader->loadShaders("shaders/dirlight_shaders/dirlight_depth.vs", "shaders/dirlight_shaders/dirlight_depth_opaque.fs");
  dirlight_depth_full_transparency_shader->loadShaders("shaders/dirlight_shaders/dirlight_depth.vs", "shaders/dirlight_shaders/dirlight_depth_full_transparency.fs");
  // dirlight_depth_partial_transparency_shader->loadShaders("shaders/dirlight_shaders/dirlight_depth.vs", "shaders/dirlight_shaders/dirlight_depth_partial_transparency.fs");

  pointlight_depth_opaque_shader->loadShaders("shaders/pointlight_shaders/pointlight_depth.vs", "shaders/pointlight_shaders/pointlight_depth_opaque.fs", "shaders/pointlight_shaders/pointlight_depth.gs");
  pointlight_depth_full_transparency_shader->loadShaders("shaders/pointlight_shaders/pointlight_depth.vs", "shaders/pointlight_shaders/pointlight_depth_full_transparency.fs", "shaders/pointlight_shaders/pointlight_depth.gs");
  // pointlight_depth_partial_transparency_shader->loadShaders("shaders/pointlight_shaders/pointlight_depth.vs", "shaders/pointlight_shaders/pointlight_depth_partial_transparency.fs", "shaders/pointlight_shaders/pointlight_depth.gs");

  object_opaque_shader->loadShaders("shaders/object_shaders/object.vs", "shaders/object_shaders/object_opaque.fs");
  object_full_transparency_shader->loadShaders("shaders/object_shaders/object.vs", "shaders/object_shaders/object_full_transparency.fs");
  // object_partial_transparency_shader->loadShaders("shaders/object_shaders/object.vs", "shaders/object_shaders/object_partial_transparency.fs");

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
  scene->render_dirlights_shadow_map(dirlight_depth_opaque_shader, dirlight_depth_full_transparency_shader, nullptr);

  // Draw the scene to the pointlight's depth buffer
  scene->render_pointlights_shadow_map(pointlight_depth_opaque_shader, pointlight_depth_full_transparency_shader, nullptr);

  // Draw the scene to our framebuffer
  //glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
  glViewport(0, 0, width(), height());
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glm::mat4 view = camera->view_matrix();



  if (scene->display_type == POINTLIGHT_DEPTH) {
    glDepthMask(GL_FALSE);

    // Draw the skybox
    skybox_shader->use();
    skybox_shader->setMat4("view", glm::mat4(glm::mat3(view)));
    skybox_shader->setInt("mode", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->get_pointlights()[0]->depth_cubemap);
    skybox_shader->setInt("skybox", 0);
    scene->skybox->draw(skybox_shader, false);

    glDepthMask(GL_TRUE);

  } else if (scene->display_type != SUNLIGHT_DEPTH) {
    glDepthMask(GL_FALSE);

    // Draw the skybox
    skybox_shader->use();
    skybox_shader->setMat4("view", glm::mat4(glm::mat3(view)));
    skybox_shader->setInt("mode", 0);
    scene->draw_skybox(skybox_shader);

    glDepthMask(GL_TRUE);

    // Draw the light
    light_shader->use();
    light_shader->setMat4("view", view);
    scene->draw_dirlight(light_shader);
    scene->draw_light(light_shader);

    // Draw the objects
    object_opaque_shader->use();
    object_opaque_shader->setVec3("camera_position", camera->position);
    object_opaque_shader->setMat4("view", view);

    int texture_unit = 0;
    texture_unit = scene->set_skybox_settings("skybox", object_opaque_shader, texture_unit);
    texture_unit = scene->set_dirlight_settings("dirlights", object_opaque_shader, texture_unit);
    texture_unit = scene->set_light_settings("lights", object_opaque_shader, texture_unit);

    object_full_transparency_shader->use();
    object_full_transparency_shader->setVec3("camera_position", camera->position);
    object_full_transparency_shader->setMat4("view", view);

    texture_unit = 0;
    texture_unit = scene->set_skybox_settings("skybox", object_full_transparency_shader, texture_unit);
    texture_unit = scene->set_dirlight_settings("dirlights", object_full_transparency_shader, texture_unit);
    texture_unit = scene->set_light_settings("lights", object_full_transparency_shader, texture_unit);

    scene->draw_objects(object_opaque_shader, object_full_transparency_shader, nullptr, true, texture_unit);
  }

  // Combine the scene into the scene framebuffer (so post-processing can be done on the entire scene)
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);

  scene_shader->use();
  scene_shader->setFloat("bloom_threshold_upper", scene->bloom_threshold_upper);
  scene_shader->setFloat("bloom_threshold_lower", scene->bloom_threshold_lower);
  scene_shader->setInt("bloom_interpolation", scene->bloom_interpolation);

  switch (scene->display_type) {
    case SUNLIGHT_DEPTH:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene->get_dirlights()[1]->depth_map);
      scene_shader->setInt("screen_texture", 0);
      scene_shader->setBool("greyscale", true);
      break;
    case POINTLIGHT_DEPTH:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      scene_shader->setBool("greyscale", true);
      break;
    case BLOOM:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      scene_shader->setBool("greyscale", false);
      break;
    case BRIGHT:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      scene_shader->setBool("greyscale", false);
      break;
    default:
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, colorbuffers[0]);
      scene_shader->setInt("screen_texture", 0);
      scene_shader->setBool("greyscale", false);
      break;
  }

  framebuffer_quad->draw(scene_shader, false);

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

    framebuffer_quad->draw(gaussian_blur_shader, false);

    horizontal = !horizontal;
  }

  glBlendFunc(GL_ONE, GL_ZERO);

  glViewport(0, 0, width(), height());
  glBindFramebuffer(GL_FRAMEBUFFER, scene->antialiasing==FXAA ? post_processing_framebuffer : qt_framebuffer);
  glClear(GL_COLOR_BUFFER_BIT);

  post_processing_shader->use();

  switch (scene->display_type) {
    case SCENE:
      post_processing_shader->setBool("do_bloom", true);
      post_processing_shader->setFloat("bloom_multiplier", scene->bloom_multiplier);
      post_processing_shader->setFloat("bloom_offset", scene->bloom_offset);

      post_processing_shader->setBool("do_exposure", true);
      post_processing_shader->setFloat("exposure", camera->exposure);

      post_processing_shader->setBool("do_gamma_correction", false);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      post_processing_shader->setInt("bloom_texture", 1);
      break;
    case BLOOM:
      post_processing_shader->setBool("do_bloom", false);
      post_processing_shader->setBool("do_exposure", false);
      post_processing_shader->setBool("do_gamma_correction", true);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bloom_colorbuffer);
      post_processing_shader->setInt("screen_texture", 0);
      break;
    case BRIGHT:
      post_processing_shader->setBool("do_bloom", false);
      post_processing_shader->setBool("do_exposure", false);
      post_processing_shader->setBool("do_gamma_correction", true);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[1]);
      post_processing_shader->setInt("screen_texture", 0);
      break;
    default:
      post_processing_shader->setBool("do_bloom", false);
      post_processing_shader->setBool("do_exposure", false);
      post_processing_shader->setBool("do_gamma_correction", true);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, scene_colorbuffers[0]);
      post_processing_shader->setInt("screen_texture", 0);
      break;
  }

  framebuffer_quad->draw(post_processing_shader, false);

  if (scene->antialiasing == FXAA) {
    glBindFramebuffer(GL_FRAMEBUFFER, qt_framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    antialiasing_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, post_processing_colorbuffer);
    antialiasing_shader->setInt("screen_texture", 0);

    framebuffer_quad->draw(antialiasing_shader, false);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  //glBindVertexArray(0);
}

void OpenGLWindow::update_perspective_matrix() {
  // Update perspective matrices
  projection = glm::perspective(glm::radians(fov), width()/float(height()), 0.1f, 100.0f);
  object_opaque_shader->use();
  object_opaque_shader->setMat4("projection", projection);
  object_full_transparency_shader->use();
  object_full_transparency_shader->setMat4("projection", projection);
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
