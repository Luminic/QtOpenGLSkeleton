#include <QGridLayout>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QRadioButton>
#include <QDebug>

#include <algorithm>

#include "Settings.h"

const char *Image_Type_String[] = {
  "Unknown",
  "Albedo Map",
  "Ambient Occlusion Map",
  "Roughness Map",
  "Metalness Map",
  "Cube Map"
};

Settings::Settings() {
  resize(600,400);
  show();

  // create_list_tab(nodes_list, nodes_list_layout, "Nodes");
  set_up_nodes_tab();
  create_list_tab(materials_list, materials_list_layout, "Materials");
}

Settings::~Settings() {}

void Settings::create_list_tab(QGroupBox*& widget, QVBoxLayout*& layout, const char* name) {
  widget = new QGroupBox(this);
  layout = new QVBoxLayout(widget);

  QScrollArea *scrolling = new QScrollArea(this);
  scrolling->setWidget(widget);
  scrolling->setWidgetResizable(true);

  addTab(scrolling, tr(name));
}

void Settings::set_up_nodes_tab() {
  nodes_model = new QStandardItemModel();
  nodes_model->setHorizontalHeaderLabels(QStringList(tr("Nodes Tree View")));

  QTreeView *tree_view = new QTreeView(this);
  tree_view->setModel(nodes_model);

  connect(tree_view, &QTreeView::clicked, this,
    [this](const QModelIndex &index){
      this->nodes_model->itemFromIndex(index)->data().value<QScrollArea*>()->show();
    }
  );

  addTab(tree_view, tr("Nodes"));
}

void Settings::set_scene(Scene *scene) {
  QWidget *Scene_widget = new QWidget(this);
  QGridLayout *Scene_layout = new QGridLayout(Scene_widget);

  QGroupBox *Post_Processing_box = new QGroupBox(this);
  QGridLayout *Post_Processing_layout = new QGridLayout(Post_Processing_box);
  create_option_group("Bloom Threshold Lower:", &scene->bloom_threshold_lower, 0.0, 5.0, 0.05, 2, Post_Processing_box, Post_Processing_layout, 0);
  create_option_group("Bloom Threshold Upper:", &scene->bloom_threshold_upper, 0.0, 5.0, 0.05, 2, Post_Processing_box, Post_Processing_layout, 2);
  create_option_group("Bloom Interpolation:", &scene->bloom_interpolation, 0.0, 5.0, 1.0, 0, Post_Processing_box, Post_Processing_layout, 4);
  create_option_group("Bloom Multiplier:", &scene->bloom_multiplier, 0.0, 5.0, 0.05, 2, Post_Processing_box, Post_Processing_layout, 6);
  create_option_group("Bloom Offset:", &scene->bloom_offset, -2.0, 6.0, 0.1, 1, Post_Processing_box, Post_Processing_layout, 8);
  create_option_group("Bloom Applications:", &scene->bloom_applications, 1.0, 10.0, 1.0, 0, Post_Processing_box, Post_Processing_layout, 10);
  Scene_layout->addWidget(Post_Processing_box, 0, 0, -1, 1);

  QGroupBox *Sky_box = new QGroupBox(this);
  QGridLayout *Sky_layout = new QGridLayout(Sky_box);
  create_option_group("Skybox Multiplier:", &scene->skybox_multiplier, 0.0, 10.0, 0.5, 1, Sky_box, Sky_layout, 0);
  Scene_layout->addWidget(Sky_box, 0, 1);

  QGroupBox *Misc_box = new QGroupBox(this);
  QGridLayout *Misc_layout = new QGridLayout(Misc_box);
  create_option_group("Display Type:", &scene->display_type, 0.0, 5.0, 1.0, 0, Misc_box, Misc_layout, 0);
  Scene_layout->addWidget(Misc_box, 1, 1);

  QGroupBox *AA_box = new QGroupBox(tr("Anti-Aliasing"), this);
  QGridLayout *AA_layout = new QGridLayout(AA_box);
  QRadioButton *none_button = new QRadioButton("None", AA_box);
  connect(none_button, &QRadioButton::clicked, this, [=](){scene->antialiasing=NONE;});
  AA_layout->addWidget(none_button, 0, 0);
  QRadioButton *fxaa_button = new QRadioButton("FXAA", AA_box);
  connect(fxaa_button, &QRadioButton::clicked, this, [=](){scene->antialiasing=FXAA;});
  AA_layout->addWidget(fxaa_button, 1, 0);
  switch (scene->antialiasing) {
    case FXAA:
      fxaa_button->setChecked(true);
      break;
    default:
      none_button->setChecked(true);
      break;
  }
  Scene_layout->addWidget(AA_box, 2, 1);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Scene_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr("Scene"));
}

void Settings::set_camera(Camera *camera) {
  QWidget *Camera_widget = new QWidget(this);
  QGridLayout *Camera_layout = new QGridLayout(Camera_widget);

  QGroupBox *Misc_box = new QGroupBox(this);
  QGridLayout *Misc_layout = new QGridLayout(Misc_box);
  create_option_group("Mouse Sensitivity:", &camera->mouse_sensitivity, 0.0, 1.0, 0.05, 2, Misc_box, Misc_layout, 0);
  create_option_group("Maximum Speed:", &camera->max_movement_speed, 0.0, 10.0, 0.1, 1, Misc_box, Misc_layout, 2);
  create_option_group("Acceleration:", &camera->acceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 4);
  create_option_group("Deceleration:", &camera->deceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 6);
  create_option_group("Exposure:", &camera->exposure, 0.0, 20.0, 0.1, 2, Misc_box, Misc_layout, 8);
  Camera_layout->addWidget(Misc_box, 0, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Camera_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr("Camera"));
}

std::vector<Material*> Settings::get_node_materials(Node *node) {
  // Probably quite inefficient but it should only be used a couple of times at the beginning
  std::vector<Material*> materials;
  for (auto mesh_ptr : node->meshes) {
    if (mesh_ptr->material != nullptr && std::find(materials.begin(), materials.end(), mesh_ptr->material) == materials.end())
        materials.push_back(mesh_ptr->material);
  }
  for (auto node_ptr : node->child_nodes) {
    std::vector<Material*> cn_mats = get_node_materials(node_ptr.get());
    for (auto mat_ptr : cn_mats) {
      if (mat_ptr != nullptr && std::find(materials.begin(), materials.end(), mat_ptr) == materials.end())
        materials.push_back(mat_ptr);
    }
  }

  return materials;
}

void Settings::set_node(Node* node, QStandardItem* parent) {
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


  QGroupBox *Material_box = new QGroupBox(tr("Materials"), this);
  QVBoxLayout *Material_layout = new QVBoxLayout(Material_box);
  for (auto material_ptr : get_node_materials(node)) {
    QPushButton *material_jump = new QPushButton(Material_box);
    if (material_ptr->textures.size() >= 1)
      material_jump->setIcon(QIcon(material_ptr->textures[0].path.c_str()));
    material_jump->setText(tr(material_ptr->name.c_str()));
    connect(material_jump, &QPushButton::clicked, this, [=](){materials[material_ptr->index]->show();});
    Material_layout->addWidget(material_jump);
  }
  Node_layout->addWidget(Material_box, 1, 1);

  QScrollArea* Scrolling = new QScrollArea(this);
  Scrolling->setWindowFlags(Qt::Window);
  Scrolling->setWindowTitle(node->name.c_str());
  Scrolling->setWidget(Node_widget);
  Scrolling->setWidgetResizable(true);

  QStandardItem* item = new QStandardItem(QString(tr(node->name.c_str())));
  QVariant item_data;
  item_data.setValue(Scrolling);
  item->setData(item_data);

  if (parent==nullptr) {
    nodes_model->invisibleRootItem()->appendRow(item);
  } else {
    parent->appendRow(item);
  }

  for (auto child : node->get_child_nodes()) {
    set_node(child.get(), item); // Recursively build the node tree
  }
}

void Settings::set_point_light(PointLight *point_light) {
  QWidget *Light_widget = new QWidget(this);
  QGridLayout *Light_layout = new QGridLayout(Light_widget);

  QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Color_Layout = new QGridLayout(Color_Box);
  create_option_group("R:", &point_light->color.r, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &point_light->color.g, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 2);
  create_option_group("B:", &point_light->color.b, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 4);
  Light_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Lighting_Box = new QGroupBox(tr("Lighting"), this);
  QGridLayout *Lighting_Layout = new QGridLayout(Lighting_Box);
  create_option_group("Ambient:", &point_light->ambient, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 0);
  create_option_group("Diffuse:", &point_light->diffuse, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 2);
  create_option_group("Specular:", &point_light->specular, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 4);
  Light_layout->addWidget(Lighting_Box, 0, 1);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &point_light->position.x, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 0);
  create_option_group("Y:", &point_light->position.y, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 2);
  create_option_group("Z:", &point_light->position.z, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 4);
  Light_layout->addWidget(Position_box, 1, 0);

  QGroupBox *Samples_Box = new QGroupBox(tr("Samples"), this);
  QGridLayout *Samples_Layout = new QGridLayout(Samples_Box);
  create_option_group("Samples:", &point_light->samples, 1.0, 26.0, 1.0, 0, Samples_Box, Samples_Layout, 0);
  create_option_group("Sample Radius:", &point_light->sample_radius, 0.0, 1.0, 0.01, 2, Samples_Box, Samples_Layout, 2);
  Light_layout->addWidget(Samples_Box, 1, 1);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Light_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(point_light->name.c_str()));
}

void Settings::set_dirlight(DirectionalLight *dirlight) {
  QWidget *Light_widget = new QWidget(this);
  QGridLayout *Light_layout = new QGridLayout(Light_widget);

  QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Color_Layout = new QGridLayout(Color_Box);
  create_option_group("R:", &dirlight->color.r, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &dirlight->color.g, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 2);
  create_option_group("B:", &dirlight->color.b, 0.0, 50.0, 0.1, 2, Color_Box, Color_Layout, 4);
  Light_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Lighting_Box = new QGroupBox(tr("Lighting"), this);
  QGridLayout *Lighting_Layout = new QGridLayout(Lighting_Box);
  create_option_group("Ambient:", &dirlight->ambient, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 0);
  create_option_group("Diffuse:", &dirlight->diffuse, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 2);
  create_option_group("Specular:", &dirlight->specular, 0.0, 5.0, 0.1, 1, Lighting_Box, Lighting_Layout, 4);
  Light_layout->addWidget(Lighting_Box, 0, 1);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &dirlight->position.x, -100.0, 100.0, 1, 1, Position_box, Position_layout, 0);
  create_option_group("Y:", &dirlight->position.y, -100.0, 100.0, 1, 1, Position_box, Position_layout, 2);
  create_option_group("Z:", &dirlight->position.z, -100.0, 100.0, 1, 1, Position_box, Position_layout, 4);
  Light_layout->addWidget(Position_box, 1, 0);

  QGroupBox *Direction_box = new QGroupBox(tr("Direction"), this);
  QGridLayout *Direction_layout = new QGridLayout(Direction_box);
  create_option_group("X:", &dirlight->direction.x, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 0);
  create_option_group("Y:", &dirlight->direction.y, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 2);
  create_option_group("Z:", &dirlight->direction.z, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 4);
  Light_layout->addWidget(Direction_box, 1, 1);

  QGroupBox *View_box = new QGroupBox(tr("View"), this);
  QGridLayout *View_layout = new QGridLayout(View_box);
  create_option_group("X View Size:", &dirlight->x_view_size, 1.0, 50.0, 1.0, 0, View_box, View_layout, 0);
  create_option_group("Y View Size:", &dirlight->y_view_size, 1.0, 50.0, 1.0, 0, View_box, View_layout, 2);
  create_option_group("Near Plane:", &dirlight->near_plane, 0.01, 10.0, 0.1, 2, View_box, View_layout, 4);
  create_option_group("Far Plane:", &dirlight->far_plane, 0.1, 500.0, 1.0, 2, View_box, View_layout, 6);
  Light_layout->addWidget(View_box, 2, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Light_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(dirlight->name.c_str()));
}

void Settings::set_material(Material* material) {
  QWidget *Material_widget = new QWidget(this);
  QGridLayout *Material_layout = new QGridLayout(Material_widget);

  QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Color_Layout = new QGridLayout(Color_Box);
  create_option_group("R:", &material->color.r, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &material->color.g, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 2);
  create_option_group("B:", &material->color.b, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 4);
  Material_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Misc_box = new QGroupBox(tr("Material Properties"), this);
  QGridLayout *Misc_layout = new QGridLayout(Misc_box);
  create_option_group("Ambient:", &material->ambient, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 0);
  create_option_group("Diffuse:", &material->diffuse, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 2);
  create_option_group("Specular:", &material->specular, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 4);
  create_option_group("Roughness:", &material->roughness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 6);
  create_option_group("Metalness:", &material->metalness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 8);
  Material_layout->addWidget(Misc_box, 0, 1);

  if (material->textures.size() >= 1) {
    QTabWidget *Image_container = new QTabWidget(this);
    for (auto texture : material->textures) {
      QLabel *texture_label = new QLabel(Image_container);
      QPixmap texture_image(texture.path.c_str());
      texture_label->setPixmap(texture_image.scaled(500, 500, Qt::KeepAspectRatio));
      Image_container->addTab(texture_label, tr(Image_Type_String[texture.type]));
    }
    Material_layout->addWidget(Image_container, 1, 0, 1, -1);
  }

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWindowFlags(Qt::Window);
  Scrolling->setWindowTitle(material->name.c_str());
  Scrolling->setWidget(Material_widget);
  Scrolling->setWidgetResizable(true);

  QPushButton* material_button = new QPushButton(materials_list);
  if (material->textures.size() >= 1)
    material_button->setIcon(QIcon(material->textures[0].path.c_str()));
  material_button->setText(tr(material->name.c_str()));
  connect(material_button, &QPushButton::clicked, this, [Scrolling](){Scrolling->show();});

  material->index = materials.size();
  materials.push_back(Scrolling);
  materials_list_layout->addWidget(material_button);
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
