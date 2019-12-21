#include <QGridLayout>
#include <QScrollArea>
#include <QDebug>

#include "Settings.h"

Settings::Settings() {
  resize(600,400);
  show();
}

Settings::~Settings() {}

void Settings::set_scene(Scene *scene, const char *name) {

}

void Settings::set_camera(Camera *camera, const char *name) {
  QWidget *Camera_widget = new QWidget(this);
  QGridLayout *Camera_layout = new QGridLayout(Camera_widget);

  QGroupBox *Misc_box = new QGroupBox(this);
  QGridLayout *Misc_layout = new QGridLayout(Misc_box);
  create_option_group("Mouse Sensitivity:", &camera->mouse_sensitivity, 0.0, 1.0, 0.05, 2, Misc_box, Misc_layout, 0);
  create_option_group("Maximum Speed:", &camera->max_movement_speed, 0.0, 10.0, 0.1, 1, Misc_box, Misc_layout, 2);
  create_option_group("Acceleration:", &camera->acceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 4);
  create_option_group("Deceleration:", &camera->deceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 6);
  Camera_layout->addWidget(Misc_box, 0, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Camera_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));
}

void Settings::set_node(Node *node, const char *name) {
  QWidget *Node_widget = new QWidget(this);
  QGridLayout *Node_layout = new QGridLayout(Node_widget);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &node->position.x, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 0);
  create_option_group("Y:", &node->position.y, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 2);
  create_option_group("Z:", &node->position.z, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 4);
  Node_layout->addWidget(Position_box, 0, 0);

  QGroupBox *Scale_box = new QGroupBox(tr("Scale"), this);
  QGridLayout *Scale_layout = new QGridLayout(Scale_box);
  create_option_group("X:", &node->scale.x, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 0);
  create_option_group("Y:", &node->scale.y, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 2);
  create_option_group("Z:", &node->scale.z, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 4);
  Node_layout->addWidget(Scale_box, 0, 1);

  QGroupBox *Rotation_box = new QGroupBox(tr("Rotation"), this);
  QGridLayout *Rotation_layout = new QGridLayout(Rotation_box);
  create_option_group("Yaw:", &node->rotation.x, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 0);
  create_option_group("Pitch:", &node->rotation.y, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 2);
  create_option_group("Roll:", &node->rotation.z, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 4);
  Node_layout->addWidget(Rotation_box, 1, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Node_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));
}

void Settings::set_point_light(PointLight *point_light, const char *name) {
  QWidget *Light_widget = new QWidget(this);
  QGridLayout *Light_layout = new QGridLayout(Light_widget);

  QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Color_Layout = new QGridLayout(Color_Box);
  create_option_group("R:", &point_light->color.r, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &point_light->color.g, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 2);
  create_option_group("B:", &point_light->color.b, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 4);
  Light_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &point_light->position.x, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 0);
  create_option_group("Y:", &point_light->position.y, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 2);
  create_option_group("Z:", &point_light->position.z, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 4);
  Light_layout->addWidget(Position_box, 0, 2);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Light_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));
}

void Settings::set_material(Material *material, const char *name) {
  QWidget *Material_widget = new QWidget(this);
  QGridLayout *Material_layout = new QGridLayout(Material_widget);

  QGroupBox *Albedo_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Albedo_Layout = new QGridLayout(Albedo_Box);
  create_option_group("R:", &material->albedo.r, 0.0, 1.0, 0.1, 2, Albedo_Box, Albedo_Layout, 0);
  create_option_group("G:", &material->albedo.g, 0.0, 1.0, 0.1, 2, Albedo_Box, Albedo_Layout, 2);
  create_option_group("B:", &material->albedo.b, 0.0, 1.0, 0.1, 2, Albedo_Box, Albedo_Layout, 4);
  Material_layout->addWidget(Albedo_Box, 0, 0);

  QGroupBox *Misc_box = new QGroupBox(this);
  QGridLayout *Misc_layout = new QGridLayout(Misc_box);
  create_option_group("Roughness:", &material->roughness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 0);
  create_option_group("Metalness:", &material->metalness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 2);
  Material_layout->addWidget(Misc_box, 0, 1);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Material_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));
}

template <typename T>
QWidget * Settings::create_option_group(
  const char *name, T *option,
  double min_val, double max_val, double step, int decimals,
  QWidget *options_box, QGridLayout *options_layout, int y_pos
) {
  if (options_box == nullptr) {
    options_box = new QWidget(this);
    options_layout = new QGridLayout(options_box);
  }

  QLabel *options_label = new QLabel(tr(name), options_box);
  QDoubleSpinBox *options_spinbox = new QDoubleSpinBox(options_box);
  QSlider *options_slider = new QSlider(Qt::Horizontal, options_box);

  // QSlider only works with ints so the doubles need to be converted first
  int conversion_factor = glm::pow(10, decimals);

  options_slider->setRange(int(min_val*conversion_factor), int(max_val*conversion_factor));
  options_slider->setSingleStep(int(step*conversion_factor));

  options_spinbox->setRange(min_val, max_val);
  options_spinbox->setSingleStep(step);
  options_spinbox->setDecimals(decimals);

  // Connect slider to spinbox
  connect(options_slider, &QSlider::valueChanged, this, [=](int value){
    options_spinbox->setValue(double(value)/conversion_factor);
  });
  // Connect spinbox to slider
  connect(options_spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value){
    options_slider->setValue(int(value*conversion_factor));
  });

  // Keep the default value set in the constructor of camera
  options_spinbox->setValue(*option);

  // Connect spinbox to the value
  connect(options_spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value){
    (*option) = value;
  });

  options_layout->addWidget(options_label, y_pos, 0, 1, 1);
  options_layout->addWidget(options_spinbox, y_pos, 1, 1, 1);
  options_layout->addWidget(options_slider, y_pos+1, 0, 1, -1);

  return options_box;
}
