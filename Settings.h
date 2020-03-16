#ifndef SETTINGS_H
#define SETTINGS_H

#include <QTabWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>

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

  void set_scene(Scene *scene);
  void set_camera(Camera *camera);
  void set_node(Node *node);
  void set_point_light(PointLight *point_light);
  void set_dirlight(DirectionalLight *sunlight);
  void set_material(Material *material);

  // Helper functions
  std::vector<Material*> get_node_materials(Node *node);

  template <typename T>
  QWidget * create_option_group(
    const char *name, T *option,
    double min_val, double max_val, double step, int decimals,
    QWidget *options_box=nullptr, QGridLayout *options_layout=nullptr, int y_pos=0
  );

private:
  void create_list_tab(QGroupBox*& widget, QVBoxLayout*& layout, const char* name);
  // Node tab
  std::vector<QScrollArea*> nodes;
  QGroupBox* nodes_list = nullptr;
  QVBoxLayout* nodes_list_layout = nullptr;
  // Materials tab
  std::vector<QScrollArea*> materials;
  QGroupBox* materials_list = nullptr;
  QVBoxLayout* materials_list_layout = nullptr;
};

#endif
