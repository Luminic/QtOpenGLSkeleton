#ifndef SETTINGS_H
#define SETTINGS_H

#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QDebug>

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Node.h"
#include "RootNode.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "Utility.h"

class Settings : public QTabWidget {
  Q_OBJECT

public:
  Settings(QWidget* parent=nullptr);
  ~Settings();

  void update_settings() {emit updating();}

  void set_scene(Scene *scene);
  void set_camera(Camera *camera);
  QStandardItem* set_node(Node* node, QStandardItem* parent=nullptr);
  QStandardItem* set_mesh(Mesh* mesh);
  QStandardItem* set_material(Material* material);
  QScrollArea* set_animation(NodeAnimation* animation);
  QScrollArea* set_animation_channel(NodeAnimationChannel* animation);
  void set_point_light(PointLight *point_light);
  void set_dirlight(DirectionalLight *sunlight);

  // Helper functions
  std::vector<Material*> get_node_materials(Node *node);

signals:
  void updating(); // Only for internal use (with lambdas)

private:
  void create_list_tab(QGroupBox*& widget, QVBoxLayout*& layout, const char* name);
  // Node tab
  void set_up_nodes_tab();
  QStandardItemModel* nodes_model = nullptr;
  std::unordered_map<Node*, QScrollArea*> loaded_nodes;

  // Meshes
  std::unordered_map<Mesh*, QScrollArea*> loaded_meshes;

  // Materials tab
  std::unordered_map<Material*, QScrollArea*> loaded_materials;
  QGroupBox* materials_list = nullptr;
  QVBoxLayout* materials_list_layout = nullptr;

  // Icons
  void load_icons();
  std::unordered_map<const char*, QIcon> icons;

  // Helper function to quickly make the options
  template <typename T>
  QWidget* create_option_group(
    const char *name, T *option,
    double min_val, double max_val, double step, int decimals,
    QWidget *parent, QGridLayout *layout, int y_pos
  );
};

#endif
