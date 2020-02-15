#ifndef SETTINGS_H
#define SETTINGS_H

#include <QTabWidget>
#include <QGridLayout>
#include <QGroupBox>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Node.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "Material.h"

class Settings : public QTabWidget {
  Q_OBJECT

public:
  Settings();
  ~Settings();

  void set_scene(Scene *scene, const char *name="Scene");
  void set_camera(Camera *camera, const char *name="Camera");
  void set_node(Node *node, const char *name="Node");
  void set_point_light(PointLight *point_light, const char *name="Point Light");
  void set_dirlight(DirectionalLight *sunlight, const char *name="Directional Light");
  void set_material(Material *material, const char *name="Material");

  // Helper functions
  std::vector<Material*> get_node_materials(Node *node);

  template <typename T>
  QWidget * create_option_group(
    const char *name, T *option,
    double min_val, double max_val, double step, int decimals,
    QWidget *options_box=nullptr, QGridLayout *options_layout=nullptr, int y_pos=0
  );

};

#endif
