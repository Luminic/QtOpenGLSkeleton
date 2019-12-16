#include <QDebug>
#include <QGridLayout>

#include <unordered_set>

#include "Camera.h"

Camera::Camera(glm::vec3 position, float yaw) {
  this->position = position;
  world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  up = world_up;

  velocity = glm::vec3(0.0f,0.0f,0.0f);

  this->yaw = yaw;
  pitch = 0.0f;

  update_vectors();
}

Camera::~Camera() {}

void Camera::initialize_camera(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time) {
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

void Camera::update_position() {
  // Deceleration
    // x-z deceleration
  if (glm::length(glm::vec2(velocity.x,velocity.z)) >= deceleration*(*delta_time)/1000.0f) {
    glm::vec2 norm = glm::normalize(glm::vec2(velocity.x,velocity.z));
    velocity.x -= norm.x*deceleration*(*delta_time)/1000.0f;
    velocity.z -= norm.y*deceleration*(*delta_time)/1000.0f;
  } else {
    velocity.x = 0;
    velocity.z = 0;
  }
    // y deceleration (separated becase I said so)
  if (glm::abs(velocity.y) >= deceleration*(*delta_time)/1000.0f) velocity.y -=
                velocity.y>0 ? deceleration*(*delta_time)/1000.0f :
                              -deceleration*(*delta_time)/1000.0f;
  else velocity.y = 0;

  // Keyboard input
  if (keys_pressed->find(Qt::Key_W) != keys_pressed->end()) {
    velocity += glm::vec3(front.x,0.0f,front.z) * (acceleration * (*delta_time)/1000.0f);
  } if (keys_pressed->find(Qt::Key_S) != keys_pressed->end()) {
    velocity -= glm::vec3(front.x,0.0f,front.z) * (acceleration * (*delta_time)/1000.0f);
  } if (keys_pressed->find(Qt::Key_A) != keys_pressed->end()) {
    velocity -= right * (acceleration * (*delta_time)/1000.0f);
  } if (keys_pressed->find(Qt::Key_D) != keys_pressed->end()) {
    velocity += right * (acceleration * (*delta_time)/1000.0f);
  } if (keys_pressed->find(Qt::Key_Space) != keys_pressed->end()) {
    velocity += world_up * (acceleration * (*delta_time)/1000.0f);
  } if (keys_pressed->find(Qt::Key_Shift) != keys_pressed->end()) {
    velocity -= world_up * (acceleration * (*delta_time)/1000.0f);
  }

  // cap x-z acceleration
  if (glm::length(glm::vec2(velocity.x,velocity.z)) > max_movement_speed*(*delta_time)/1000.0f) {
    glm::vec2 norm = glm::normalize(glm::vec2(velocity.x,velocity.z));
    velocity.x = norm.x * max_movement_speed*(*delta_time)/1000.0f;
    velocity.z = norm.y * max_movement_speed*(*delta_time)/1000.0f;
  }
  // cap y acceleration
  if (glm::abs(velocity.y) > max_movement_speed*(*delta_time)/1000.0f)
    velocity.y = max_movement_speed*(*delta_time)/1000.0f * (velocity.y>0? 1:-1);

  // Acceleration
  position += velocity * float((*delta_time));
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

// Getters
glm::vec3 Camera::get_position() {return position;}
glm::vec3 Camera::get_front() {return front;}
glm::vec3 Camera::get_up() {return up;}
glm::vec3 Camera::get_velocity() {return velocity;}
float Camera::get_yaw() {return yaw;}
float Camera::get_pitch() {return pitch;}
float Camera::get_max_movement_speed() {return max_movement_speed;}
float Camera::get_acceleration() {return acceleration;}
float Camera::get_deceleration() {return deceleration;}
float Camera::get_mouse_sensitivity() {return mouse_sensitivity;}

// Setters (which are slots)
void Camera::set_velocity(glm::vec3 new_velocity) {velocity = new_velocity;}
void Camera::set_max_movement_speed(float speed) {max_movement_speed=speed;}
void Camera::set_acceleration(float acceleration) {this->acceleration=acceleration;}
void Camera::set_deceleration(float deceleration) {this->deceleration=deceleration;}
void Camera::set_mouse_sensitivity(float sensitivity) {mouse_sensitivity=sensitivity;}
