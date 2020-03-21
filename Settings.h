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

#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"
#include "Camera.h"
#include "Node.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "Material.h"

class Slider_Spinbox_Group : public QWidget {
  Q_OBJECT;

public:
  Slider_Spinbox_Group(const QString& name, QWidget* parent=nullptr) : QWidget(parent) {
    this->name = name;
    init();
  }
  Slider_Spinbox_Group(double minimum, double maximum, double step, int decimals, const QString& name, QWidget* parent=nullptr) : QWidget(parent) {
    this->minimum = minimum;
    this->maximum = maximum;
    this->step = step;
    this->decimals = decimals;
    this->name = name;
    init();
  }
  ~Slider_Spinbox_Group() {}

  void connect_slots_and_signals() {
    connect(slider, &QSlider::valueChanged, this,
      [this](int value){
        spinbox->setValue(double(value)/conversion_factor);
      }
    );
    connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
      [this](double value){
        slider->setValue(int(value*conversion_factor));
      }
    );
    connect(this, &Slider_Spinbox_Group::valueChanged, spinbox, &QDoubleSpinBox::setValue);
    connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Slider_Spinbox_Group::setValue);
  }

  int get_decimals() {return decimals;}
  void set_decimals(int decimals) {
    this->decimals = decimals;
    conversion_factor = glm::pow(10, decimals);
    spinbox->setDecimals(decimals);
    // Update the slider to use the new conversion factors
    set_minimum(minimum); // I'm lazy so I'll do it this way even though its inefficient
    set_maximum(maximum);
    set_step(step);
  }

  double get_minimum() {return minimum;}
  void set_minimum(double minimum) {
    this->minimum = minimum;
    spinbox->setMinimum(minimum);
    slider->setMinimum(int(minimum*conversion_factor));
  }

  double get_maximum() {return maximum;}
  void set_maximum(double maximum) {
    this->maximum = maximum;
    spinbox->setMaximum(maximum);
    slider->setMaximum(int(maximum*conversion_factor));
  }

  double get_step() {return step;}
  void set_step(double step) {
    this->step = step;
    spinbox->setSingleStep(step);
    slider->setSingleStep(int(step*conversion_factor));
  }

public slots:
  void setValue(double value) {
    if (value != this->value) {
      emit valueChanged(value);
      this->value = value;
    }
  }

signals:
  void valueChanged(double value);

private:
  void init() {
    label = new QLabel(name, this);
    spinbox = new QDoubleSpinBox(this);
    slider = new QSlider(Qt::Horizontal, this);

    layout = new QGridLayout(this);
    layout->addWidget(label,   0, 0, 1, 1);
    layout->addWidget(spinbox, 0, 1, 1, 1);
    layout->addWidget(slider,  1, 0, 1,-1);

    set_decimals(decimals);
    set_minimum(minimum);
    set_maximum(maximum);
    set_step(step);

    connect_slots_and_signals();
  }

  double value=0;
  QString name;

  int decimals=0;
  int conversion_factor=1;
  double minimum=0;
  double maximum=10;
  double step=1;

  QGridLayout* layout;
  QLabel* label;
  QDoubleSpinBox* spinbox;
  QSlider* slider;
};

class Settings : public QTabWidget {
  Q_OBJECT

public:
  Settings();
  ~Settings();

  void set_scene(Scene *scene);
  void set_camera(Camera *camera);
  QStandardItem* set_node(Node* node, QStandardItem* parent=nullptr);
  QStandardItem* set_mesh(Mesh* mesh);
  QStandardItem* set_material(Material* material);
  void set_point_light(PointLight *point_light);
  void set_dirlight(DirectionalLight *sunlight);

  // Helper functions
  std::vector<Material*> get_node_materials(Node *node);

private:
  void create_list_tab(QGroupBox*& widget, QVBoxLayout*& layout, const char* name);
  // Node tab
  void set_up_nodes_tab();
  QStandardItemModel* nodes_model = nullptr;
  std::unordered_map<const char*, QScrollArea*> loaded_nodes;

  // Meshes
  std::unordered_map<const char*, QScrollArea*> loaded_meshes;

  // Materials tab
  std::unordered_map<const char*, QScrollArea*> loaded_materials;
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
