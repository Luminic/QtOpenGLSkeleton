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
  Camera(glm::vec3 position=glm::vec3(0.0f,0.0f,-3.0f), float yaw=0, float pitch=0);
  ~Camera();

  void initialize_camera(std::unordered_set<int> *keys_pressed, QPoint *mouse_movement, int *delta_time);

  void update_cam();

  glm::mat4 view_matrix();

  // Getters
  glm::vec3 get_front();
  glm::vec3 get_up();

  glm::vec3 position;
  float mouse_sensitivity;
  float max_movement_speed;
  float acceleration;
  float deceleration;

  float exposure;

private:
  void update_direction();
  void update_position();
  void update_vectors();

  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 world_up;

  glm::vec3 velocity;

  float yaw;
  float pitch;

  std::unordered_set<int> *keys_pressed;
  QPoint *mouse_movement;
  int *delta_time;
};

#endif
