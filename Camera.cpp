#include <QDebug>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_set>

#include "Camera.h"

Camera::Camera(glm::vec3 position, float yaw, float pitch) :
  position(position),
  yaw(yaw),
  pitch(pitch)
{
  mouse_sensitivity = 0.05f;
  max_movement_speed = 0.5f;
  acceleration = 0.15f;
  deceleration = 0.12f;

  world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  up = world_up;

  velocity = glm::vec3(0.0f,0.0f,0.0f);

  exposure = 1.0f;

  update_vectors();
}

Camera::~Camera() {}

void Camera::initialize_camera(const std::unordered_set<int>* keys_pressed, const QPoint* mouse_movement, const int* delta_time) {
  this->keys_pressed = keys_pressed;
  this->mouse_movement = mouse_movement;
  this->delta_time = delta_time;
}

void Camera::update_cam() {
  update_direction();
  update_position();
}

void Camera::update_direction() {
  if (!mouse_movement->isNull()) {
    yaw -= mouse_movement->x() * mouse_sensitivity;
    pitch -= mouse_movement->y() * mouse_sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    else if (pitch < -89.0f) pitch = -89.0f;
    update_vectors();
  }
}

float Camera::friction_curve(float speed) {
  // friction(x) = 1/(x+1) +x -1
  //return 1.0f/(speed+1.0f) + speed - 1.0f;
  return 1.0f/(speed+1.0f);
}

float Camera::acceleration_multiplier(float speed) {
  return 1.0f/(50.0f*speed+1.0f) + 1.0f;
}

void Camera::update_position() {
  glm::vec2 current_xz_movement(0.0f);
  float current_y_movement(0.0f); // Upwards movement is calculated seperately.

  // Keyboard input
  if (keys_pressed->find(Qt::Key_W) != keys_pressed->end()) {
    current_xz_movement += glm::vec2(front.x, front.z);
  }
  if (keys_pressed->find(Qt::Key_S) != keys_pressed->end()) {
    current_xz_movement -= glm::vec2(front.x, front.z);
  }
  if (keys_pressed->find(Qt::Key_A) != keys_pressed->end()) {
    current_xz_movement -= glm::vec2(right.x, right.z);
  }
  if (keys_pressed->find(Qt::Key_D) != keys_pressed->end()) {
    current_xz_movement += glm::vec2(right.x, right.z);
  }
  if (keys_pressed->find(Qt::Key_Space) != keys_pressed->end()) {
    current_y_movement += 1.0f;
  }
  if (keys_pressed->find(Qt::Key_Shift) != keys_pressed->end()) {
    current_y_movement -= 1.0f;
  }

  if (glm::abs(current_xz_movement.x)+glm::abs(current_xz_movement.y) > 0.01f) {
    current_xz_movement = glm::normalize(current_xz_movement);
    current_xz_movement *= acceleration * (*delta_time)/1000.0f;
    current_xz_movement *= acceleration_multiplier(glm::length(glm::vec2(velocity.x,velocity.z)));
    velocity.x += current_xz_movement.x;
    velocity.z += current_xz_movement.y;
  }
  // qDebug() << velocity.x << velocity.z;
  float friction = friction_curve(glm::length(glm::vec2(velocity.x,velocity.z)));
  velocity.x *= friction;
  velocity.z *= friction;
  if (glm::length(glm::vec2(velocity.x,velocity.z)) > deceleration*(*delta_time)/1000.0f) {
    glm::vec2 norm = glm::normalize(glm::vec2(velocity.x,velocity.z));
    norm *= deceleration*(*delta_time)/1000.0f;
    velocity.x -= norm.x;
    velocity.z -= norm.y;
  } else {
    velocity.x = 0;
    velocity.z = 0;
  }

  // No need to normalize y speed as it can only be 1, -1, or 0
  velocity.y += current_y_movement * acceleration * (*delta_time)/1000.0f * acceleration_multiplier(glm::abs(velocity.y));
  velocity.y *= friction_curve(glm::abs(velocity.y));
  if (glm::abs(velocity.y) > deceleration*(*delta_time)/1000.0f) {
    velocity.y += velocity.y > 0 ? -deceleration*(*delta_time)/1000.0f : deceleration*(*delta_time)/1000.0f;
  } else {
    velocity.y = 0;
  }

  // Acceleration
  position += velocity / (glm::log((*delta_time)*100.0f) / glm::log(10.0f)) * 50.0f;
}

glm::mat4 Camera::view_matrix() {
  return glm::lookAt(position, position+front, up);
}

void Camera::update_vectors() {
  front.x = sin(glm::radians(yaw));// * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch)) / cos(glm::radians(pitch));
  front.z = cos(glm::radians(yaw));// * cos(glm::radians(pitch));
  //front = glm::normalize(front);

  right = glm::normalize(glm::cross(front, world_up));
  up = glm::normalize(glm::cross(right, front));
}

glm::vec3 Camera::get_front() {return front;}
glm::vec3 Camera::get_up() {return up;}
