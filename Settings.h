#ifndef SETTINGS_H
#define SETTINGS_H

#include <QTabWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>

#include <vector>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Node.h"
#include "Model.h"
#include "PointLight.h"

class Settings : public QTabWidget {
  Q_OBJECT

public:
  Settings(QWidget *parent=nullptr);
  ~Settings();

  void set_scene(Scene *scene, const char *name="Scene");
  void set_camera(Camera *camera, const char *name="Camera");
  void set_object(Node *object, const char *name="Node");
  void set_point_light(PointLight *point_light, const char *name="Point Light");

  template <class T>
  QGroupBox * create_3_option_group(const char *name,
    const char* label1, const char* label2, const char* label3,
    T *context, void (T::*setting1)(float), void (T::*setting2)(float), void (T::*setting3)(float),
    glm::vec3 default_val, double min_val, double max_val, double step, int decimals
  );

  template <class T, typename Y>
  QWidget * create_option_group(
    const char *name, T *context, void (T::*setting)(Y),
    double default_val, double min_val, double max_val, double step, int decimals,
    QWidget *options_box=nullptr, QGridLayout *options_layout=nullptr, int y_pos=0
  );
};

#endif
