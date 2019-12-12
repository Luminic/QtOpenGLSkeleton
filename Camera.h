#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QKeyEvent>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public QObject {
  Q_OBJECT

public:
  Camera(glm::vec3 position=glm::vec3(0.0f,0.0f,-3.0f), float yaw=0);
  ~Camera();

  void initialize_camera(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time);

  void update_cam();

  glm::mat4 view_matrix();

  // Getters
  glm::vec3 get_position();
  glm::vec3 get_front();
  glm::vec3 get_up();
  glm::vec3 get_velocity();
  float get_yaw();
  float get_pitch();
  float get_max_movement_speed();
  float get_acceleration();
  float get_deceleration();
  float get_mouse_sensitivity();

public slots: // Setters
  void set_velocity(glm::vec3 new_velocity);
  void set_max_movement_speed(float speed);
  void set_acceleration(float acceleration);
  void set_deceleration(float deceleration);
  void set_mouse_sensitivity(float sensitivity);

private:
  void update_direction();
  void update_position();
  void update_vectors();

  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 world_up;

  glm::vec3 velocity;

  float yaw;
  float pitch;

  // Initialization of these is handled by the settings class
  float max_movement_speed;
  float acceleration;
  float deceleration;
  float mouse_sensitivity;

  std::unordered_set<int> *keys_pressed;
  QPoint *mouse_movement;
  int *delta_time;
};

#endif
