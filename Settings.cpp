#include <QGridLayout>
#include <QScrollArea>
#include <QDebug>

#include "Settings.h"

Settings::Settings(QWidget *parent) {
  show();
}

Settings::~Settings() {}

void Settings::set_scene(Scene *scene, const char *name) {

}

void Settings::set_camera(Camera *camera, const char *name) {
  QWidget *Camera_widget = new QWidget(this);
  QGridLayout *Camera_layout = new QGridLayout(Camera_widget);
  create_option_group("Mouse Sensitivity:", camera, &Camera::set_mouse_sensitivity,
                0.05, 0.01, 1.0, 0.05, 2);
  create_option_group("Maximum Speed:", camera, &Camera::set_max_movement_speed, 0.5, 0.1, 10.0, 0.1, 1, Camera_widget, Camera_layout, 0);
  create_option_group("Acceleration:", camera, &Camera::set_acceleration, 0.15, 0.01, 100.0, 0.1, 2, Camera_widget, Camera_layout, 2);
  create_option_group("Deceleration:", camera, &Camera::set_deceleration, 0.08, 0.01, 100.0, 0.1, 2, Camera_widget, Camera_layout, 4);
  addTab(Camera_widget, tr(name));
}

void Settings::set_object(Node *object, const char *name) {
  /*QWidget *Object_widget = new QWidget(this);
  QGridLayout *Object_layout = new QGridLayout(Object_widget);

  QGroupBox *Scale_box = create_3_option_group("Scale", "X:", "Y:", "Z:",
    object,
    &Node::set_x_scale, &Node::set_y_scale, &Node::set_z_scale,
    object->get_scale(), 0.0, 1.0, 0.1, 3
  );

  Object_layout->addWidget(Scale_box, 0, 0);

  QGroupBox *Position_box = create_3_option_group("Position", "X:", "Y:", "Z:",
    object,
    &Node::set_x_position, &Node::set_y_position, &Node::set_z_position,
    object->get_position(), -100.0, 100.0, 0.3, 3
  );

  Object_layout->addWidget(Position_box, 1, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Object_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));*/
}

void Settings::set_point_light(PointLight *point_light, const char *name) {
  /*QWidget *Light_widget = new QWidget(this);
  QGridLayout *Light_layout = new QGridLayout(Light_widget);


  QGroupBox *Color_box = create_3_option_group("Color", "R:", "G:", "B:",
    point_light,
    &PointLight::set_red, &PointLight::set_green, &PointLight::set_blue,
    point_light->get_color(), 0.0, 1.0, 0.1, 2
  );
  Light_layout->addWidget(Color_box, 0, 0);

  QGroupBox *Position_box = create_3_option_group("Position", "X:", "Y:", "Z:",
    static_cast<Node*>(point_light),
    &Node::set_x_position, &Node::set_y_position, &Node::set_z_position,
    point_light->get_position(), -100.0, 100.0, 0.3, 3
  );
  Light_layout->addWidget(Position_box, 1, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Light_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(name));*/
}

template <class T>
QGroupBox * Settings::create_3_option_group(const char *name,
  const char* label1, const char* label2, const char* label3,
  // Class ptr and function reference
  T *context, void (T::*setting1)(float), void (T::*setting2)(float), void (T::*setting3)(float),
  // Setup values
  glm::vec3 default_val, double min_val, double max_val, double step, int decimals)
{
  QGroupBox *Options_group = new QGroupBox(tr(name), this);
  QGridLayout *Options_layout = new QGridLayout(Options_group);
  create_option_group(label1, context, setting1,
                default_val.x, min_val, max_val, step, decimals, Options_group, Options_layout, 0);
  create_option_group(label2, context, setting2,
                default_val.y, min_val, max_val, step, decimals, Options_group, Options_layout, 2);
  create_option_group(label3, context, setting3,
                default_val.z, min_val, max_val, step, decimals, Options_group, Options_layout, 4);
  return Options_group;
}

template <class T, typename Y>
QWidget * Settings::create_option_group(const char *name,
  // Class ptr and function reference
  T *context, void (T::*setting)(Y),
  // Setup values
  double default_val, double min_val, double max_val, double step, int decimals,
  // Optional: Add to existing Groupbox & Layout
  QWidget *options_box, QGridLayout *options_layout, int y_pos)
{
  if (options_box==nullptr) {
    options_box = new QWidget(this);
    options_layout = new QGridLayout(options_box);
  }
  QLabel *options_label = new QLabel(tr(name), options_box);
  QDoubleSpinBox *options_spinbox = new QDoubleSpinBox(options_box);
  QSlider *options_slider = new QSlider(Qt::Horizontal, options_box);

  // Connect spinbox to the setval function
  connect(options_spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), context, setting);
  // QSlider only works with ints so the doubles need to be converted first
  int conversion_factor = glm::pow(10, decimals);
  // Connect slider to spinbox
  connect(options_slider, &QSlider::valueChanged, this, [=](int value){
    options_spinbox->setValue(double(value)/conversion_factor);
  });
  // Connect spinbox to slider
  connect(options_spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double value){
    options_slider->setValue(int(value*conversion_factor));
  });

  options_slider->setRange(int(min_val*conversion_factor), int(max_val*conversion_factor));
  options_slider->setSingleStep(int(step*conversion_factor));

  options_spinbox->setRange(min_val, max_val);
  options_spinbox->setSingleStep(step);
  options_spinbox->setDecimals(decimals);
  options_spinbox->setValue(default_val);

  options_layout->addWidget(options_label, y_pos, 0, 1, 1);
  options_layout->addWidget(options_spinbox, y_pos, 1, 1, 1);
  options_layout->addWidget(options_slider, y_pos+1, 0, 1, -1);

  return options_box;

  //if (y_pos==0) settings_layout->addWidget(options_box);
}
