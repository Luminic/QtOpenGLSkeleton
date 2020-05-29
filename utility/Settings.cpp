#include <QApplication>
#include <QPushButton>
#include <QPixmap>
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

Settings::Settings(QWidget* parent) : QTabWidget(parent) {
  resize(600,400);
  show();

  load_icons();
  set_up_nodes_tab();
  create_list_tab(materials_list, materials_list_layout, "Materials");
}

Settings::~Settings() {}

void Settings::load_icons() {
  icons["mesh"] = QIcon("assets/textures/icons/mesh.png");
  icons["material"] = QIcon("assets/textures/icons/material.png");
}

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
      QStandardItem* item = this->nodes_model->itemFromIndex(index);
      if (item->isCheckable()) {
        if (item->data().canConvert<Node*>()) {
          item->data().value<Node*>()->set_visibility(item->checkState() == Qt::CheckState::Checked);
        }
      }
    }
  );

  connect(tree_view, &QTreeView::doubleClicked, this,
    [this](const QModelIndex &index){
      QVariant data = this->nodes_model->itemFromIndex(index)->data();
      QScrollArea* clicked_widget;
      if (data.canConvert<Node*>()) {
        clicked_widget = loaded_nodes.find(data.value<Node*>())->second;
      } else if (data.canConvert<Mesh*>()) {
        clicked_widget = loaded_meshes.find(data.value<Mesh*>())->second;
      } else if (data.canConvert<Material*>()) {
        clicked_widget = loaded_materials.find(data.value<Material*>())->second;
      } else {
        qFatal("Double click on tree view: invalid data: could not convert to a supported type");
      }
      clicked_widget->show();
      QApplication::setActiveWindow(clicked_widget);
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
  create_option_group("Bloom Threshold Upper:", &scene->bloom_threshold_upper, 0.0, 5.0, 0.05, 2, Post_Processing_box, Post_Processing_layout, 1);
  create_option_group("Bloom Interpolation:", &scene->bloom_interpolation, 0.0, 5.0, 1.0, 0, Post_Processing_box, Post_Processing_layout, 2);
  create_option_group("Bloom Multiplier:", &scene->bloom_multiplier, 0.0, 5.0, 0.05, 2, Post_Processing_box, Post_Processing_layout, 3);
  create_option_group("Bloom Offset:", &scene->bloom_offset, -2.0, 6.0, 0.1, 1, Post_Processing_box, Post_Processing_layout, 4);
  create_option_group("Bloom Applications:", &scene->bloom_applications, 1.0, 10.0, 1.0, 0, Post_Processing_box, Post_Processing_layout, 5);
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

  QGroupBox *Volumetrics_box = new QGroupBox(tr("Volumetrics_box"), this);
  QGridLayout *Volumetrics_layout = new QGridLayout(Volumetrics_box);
  create_option_group("Samples:",    &scene->volumetric_samples,    0.0, 1000.0, 1.00, 0, Volumetrics_box, Volumetrics_layout, 0);
  create_option_group("Scattering:", &scene->volumetric_scattering, 0.0,    1.0, 0.01, 2, Volumetrics_box, Volumetrics_layout, 1);
  create_option_group("Density:",    &scene->volumetric_density,    0.0,    1.0, 0.01, 2, Volumetrics_box, Volumetrics_layout, 2);
  create_option_group("Scattering Direction:", &scene->scattering_direction, -1.0, 1.0, 0.01, 2, Volumetrics_box, Volumetrics_layout, 3);
  Scene_layout->addWidget(Volumetrics_box, 3, 1);

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
  create_option_group("Maximum Speed:", &camera->max_movement_speed, 0.0, 10.0, 0.1, 1, Misc_box, Misc_layout, 1);
  create_option_group("Acceleration:", &camera->acceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 2);
  create_option_group("Deceleration:", &camera->deceleration, 0.0, 10.0, 0.1, 2, Misc_box, Misc_layout, 3);
  create_option_group("Exposure:", &camera->exposure, 0.0, 20.0, 0.1, 2, Misc_box, Misc_layout, 4);
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

QStandardItem* Settings::set_node(Node* node, QStandardItem* parent) {
  QWidget *Node_widget = new QWidget(this);
  QGridLayout *Node_layout = new QGridLayout(Node_widget);

  unsigned int y_location = 0;

  Matrix_4x4_View* node_transformation = new Matrix_4x4_View(tr("Transformation Matrix"), Node_widget);
  node_transformation->set_matrix(node->transformation);
  connect(node_transformation, &Matrix_4x4_View::value_changed, this,
    [=](const glm::mat4& new_value) {
      node->transformation = new_value;
    }
  );
  Node_layout->addWidget(node_transformation, y_location, 0);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), Node_widget);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &node->position.x, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 0);
  create_option_group("Y:", &node->position.y, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 1);
  create_option_group("Z:", &node->position.z, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 2);
  Node_layout->addWidget(Position_box, y_location++, 1);

  QGroupBox *Scale_box = new QGroupBox(tr("Scale"), Node_widget);
  QGridLayout *Scale_layout = new QGridLayout(Scale_box);
  create_option_group("X:", &node->scale.x, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 0);
  create_option_group("Y:", &node->scale.y, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 1);
  create_option_group("Z:", &node->scale.z, -50.0, 50.0, 0.5, 2, Scale_box, Scale_layout, 2);
  Node_layout->addWidget(Scale_box, y_location, 0);

  QGroupBox *Rotation_box = new QGroupBox(tr("Rotation"), Node_widget);
  QGridLayout *Rotation_layout = new QGridLayout(Rotation_box);
  create_option_group("Yaw:", &node->rotation.x, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 0);
  create_option_group("Pitch:", &node->rotation.y, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 1);
  create_option_group("Roll:", &node->rotation.z, 0.0, 360.0, 1, 1, Rotation_box, Rotation_layout, 2);
  Node_layout->addWidget(Rotation_box, y_location++, 1);


  QGroupBox *Material_box = new QGroupBox(tr("Materials"), Node_widget);
  QVBoxLayout *Material_layout = new QVBoxLayout(Material_box);
  for (auto material_ptr : get_node_materials(node)) {
    QPushButton *material_jump = new QPushButton(Material_box);
    if (material_ptr->textures.size() >= 1) {
      Q_ASSERT_X(material_ptr->textures[0].image.isNull() == false, "texture icons creation", "QImage is null");
      material_jump->setIcon(QIcon(QPixmap::fromImage(material_ptr->textures[0].image)));
    }
    material_jump->setText(tr(material_ptr->name.c_str()));
    connect(material_jump, &QPushButton::clicked, this,
      [this, material_ptr](){
        auto material_window = loaded_materials.find(material_ptr)->second;
        material_window->show();
        QApplication::setActiveWindow(material_window);
      }
    );
    Material_layout->addWidget(material_jump);
  }
  Node_layout->addWidget(Material_box, y_location++, 0, 1, -1);

  QScrollArea* Scrolling = new QScrollArea(this);
  Scrolling->setWindowFlags(Qt::Window);
  Scrolling->setWindowTitle(node->name.c_str());
  Scrolling->setWidget(Node_widget);
  Scrolling->setWidgetResizable(true);

  loaded_nodes[node] = Scrolling;

  QStandardItem* item = new QStandardItem(tr(node->name.c_str()));
  item->setCheckable(true);
  item->setCheckState(node->visible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
  QVariant item_data;
  item_data.setValue(node);
  item->setData(item_data);

  if (parent==nullptr) {
    nodes_model->invisibleRootItem()->appendRow(item);
  }

  for (auto child : node->get_child_nodes()) {
    item->appendRow(set_node(child.get(), item)); // Recursively build the node tree
  }

  for (auto mesh : node->get_meshes()) {
    item->appendRow(set_mesh(mesh.get()));
  }

  RootNode* root_node = dynamic_cast<RootNode*>(node);
  if (root_node) { // If the node is a root node
    QGroupBox* animation_box = set_root_node(root_node, Node_widget);
    if (animation_box) {
      Node_layout->addWidget(animation_box, y_location++, 0, 1, -1);
    }
  }

  return item;
}

QGroupBox* Settings::set_root_node(RootNode* root_node, QWidget* parent) {
  if (root_node->animation.size() > 0) {
    QGroupBox* animation_box = new QGroupBox(tr("Animation"), parent);
    QGridLayout* animation_layout = new QGridLayout(animation_box);

    auto animation_status_to_string = [](RootNode::Animation_Status animation_status) {
      QString animation_status_string;
      switch (animation_status) {
        case RootNode::Animation_Status::NO_ANIMATION:
        animation_status_string = "NO_ANIMATION";
        break;
        case RootNode::Animation_Status::ANIMATION_PAUSED:
        animation_status_string = "ANIMATION_PAUSED";
        break;
        case RootNode::Animation_Status::ANIMATED:
        animation_status_string = "ANIMATED";
        break;
      }
      return animation_status_string;
    };

    QLabel* animation_status_label = new QLabel(tr("Current Animation Status: ")+animation_status_to_string(root_node->get_animation_status()), animation_box);
    animation_layout->addWidget(animation_status_label, 1, 0, 1, -1);
    connect(root_node, &RootNode::animation_status_changed, this,
      [animation_status_label, animation_status_to_string] (RootNode::Animation_Status new_animation_status) {
        animation_status_label->setText(tr("Current Animation Status: ")+animation_status_to_string(new_animation_status));
      }
    );

    QLabel* animation_name_label = new QLabel(tr("Current Animation: ")+tr(root_node->get_current_animation_name().c_str()), animation_box);
    animation_layout->addWidget(animation_name_label, 2, 0, 1, -1);
    connect(root_node, &RootNode::animation_changed, this,
      [animation_name_label] (NodeAnimation* new_animation) {
        animation_name_label->setText(tr("Current Animation: ")+tr(new_animation->name.c_str()));
      }
    );

    QLabel* animation_time_label = new QLabel(tr("Animation Time: ")+QString::number(root_node->get_animation_time()), animation_box);
    animation_layout->addWidget(animation_time_label, 3, 0, 1, -1);
    connect(this, &Settings::updating, this,
      [animation_time_label, root_node] () {
        animation_time_label->setText(tr("Animation Time: ")+QString::number(root_node->get_animation_time()));
      }
    );

    QSlider* animation_time_slider = new QSlider(Qt::Horizontal, animation_box);
    animation_time_slider->setRange(0,100);
    animation_layout->addWidget(animation_time_slider, 4, 0, 1, -1);
    connect(this, &Settings::updating, this,
      [animation_time_slider, root_node] () {
        if (root_node->get_animation_status() == RootNode::Animation_Status::ANIMATED) {
          animation_time_slider->setValue(int(root_node->get_animation_time()/root_node->get_current_animation()->duration*100));
        }
      }
    );
    connect(animation_time_slider, &QSlider::valueChanged, this,
      [root_node] (int value) {
        if (root_node->get_animation_status() == RootNode::Animation_Status::ANIMATION_PAUSED) {
          root_node->set_animation_time(value/100.0f*root_node->get_current_animation()->duration);
        }
      }
    );

    QPushButton* play_pause_button = new QPushButton(tr("Play/Pause"), animation_box);
    animation_layout->addWidget(play_pause_button, 5, 0, 1, 1);
    connect(play_pause_button, &QPushButton::clicked, this,
      [root_node] () {
        if (root_node->get_animation_status() == RootNode::Animation_Status::ANIMATED) {
          root_node->stop_animation();
        } else {
          root_node->start_animation();
        }
      }
    );

    QPushButton* disable_animation_button = new QPushButton(tr("Disable Animation"), animation_box);
    animation_layout->addWidget(disable_animation_button, 5, 1, 1, 1);
    connect(disable_animation_button, &QPushButton::clicked, this,
      [root_node] () {
        root_node->disable_animation();
      }
    );

    unsigned int y_pos = 5;
    for (auto it : root_node->animation) {
      QPushButton* animation_jump = new QPushButton(animation_box);
      animation_jump->setText(tr(it.second->name.c_str()));
      animation_layout->addWidget(animation_jump, ++y_pos, 0, 1, -1);

      QScrollArea* animation_menu = set_animation(it.second);
      connect(animation_jump, &QPushButton::clicked, this,
        [animation_menu](){
          animation_menu->show();
          QApplication::setActiveWindow(animation_menu);
        }
      );
    }

    return animation_box;
  }
  return nullptr;
}

QStandardItem* Settings::set_mesh(Mesh* mesh) {
  QStandardItem* mesh_item = new QStandardItem(QString(tr(mesh->name.c_str())));
  mesh_item->setIcon(icons.find("mesh")->second);
  QVariant mesh_item_data;
  mesh_item_data.setValue(mesh);

  // Window Creation
  if (loaded_meshes.find(mesh) == loaded_meshes.end()) {
    QWidget* mesh_box = new QWidget(this);
    QGridLayout* mesh_layout = new QGridLayout(mesh_box);

    unsigned int y_location = 0;

    Tesseract* as_tesseract = dynamic_cast<Tesseract*>(mesh);
    if (as_tesseract) {
      QGroupBox* tesseract_box = set_tesseract(as_tesseract, mesh_box);
      mesh_layout->addWidget(tesseract_box, y_location++, 0, 1, -1);
    }

    if (y_location == 0) {
      QLabel* label = new QLabel(tr("Nothing to see here!"), mesh_box);
      mesh_layout->addWidget(label, y_location++, 0, 1, -1);
    }

    QScrollArea* Scrolling = new QScrollArea(this);
    Scrolling->setWindowFlags(Qt::Window);
    Scrolling->setWindowTitle(mesh->name.c_str());
    Scrolling->setWidget(mesh_box);
    Scrolling->setWidgetResizable(true);

    loaded_meshes[mesh] = Scrolling;
  }

  if (mesh->material != nullptr) {
    mesh_item->appendRow(set_material(mesh->material));
  }

  mesh_item->setData(mesh_item_data);
  return mesh_item;
}

QGroupBox* Settings::set_tesseract(Tesseract* tesseract, QWidget* parent) {
  QGroupBox* tesseract_box = new QGroupBox(tr("Tesseract Settings"), parent);
  QGridLayout* tesseract_layout = new QGridLayout(tesseract_box);

  unsigned int y_location = 0;

  QGroupBox* rotation_box = new QGroupBox(tr("Rotation Settings"), tesseract_box);
  tesseract_layout->addWidget(rotation_box, y_location++, 0, 1, -1);
  QGridLayout* rotation_layout = new QGridLayout(rotation_box);

  Slider_Spinbox_Group* angle_group = new Slider_Spinbox_Group(0, 2*3.141592654, 0.05, 2, tr("Angle"), rotation_box);
  rotation_layout->addWidget(angle_group, 0, 0, 1, -1);

  QGroupBox* rotation_plane_box = new QGroupBox(tr("Plane of Rotation"), rotation_box);
  QGridLayout* rotation_plane_layout = new QGridLayout(rotation_plane_box);
  QRadioButton *xy = new QRadioButton("XY", angle_group);
  xy->setChecked(true);
  rotation_plane_layout->addWidget(xy, 0, 0, 1, 1);
  QRadioButton *xz = new QRadioButton("XZ", angle_group);
  rotation_plane_layout->addWidget(xz, 1, 0, 1, 1);
  QRadioButton *xw = new QRadioButton("XW", angle_group);
  rotation_plane_layout->addWidget(xw, 2, 0, 1, 1);
  QRadioButton *yz = new QRadioButton("YZ", angle_group);
  rotation_plane_layout->addWidget(yz, 0, 1, 1, 1);
  QRadioButton *yw = new QRadioButton("YW", angle_group);
  rotation_plane_layout->addWidget(yw, 1, 1, 1, 1);
  QRadioButton *zw = new QRadioButton("ZW", angle_group);
  rotation_plane_layout->addWidget(zw, 2, 1, 1, 1);

  rotation_layout->addWidget(rotation_plane_box, 1, 0, 1, -1);

  QPushButton* apply_rotation_button = new QPushButton(tr("Apply Rotation"), rotation_box);
  rotation_layout->addWidget(apply_rotation_button, 2, 0, 1, -1);

  QPushButton* animate_button = new QPushButton(tr("Animate"), rotation_box);
  animate_button->setCheckable(true);
  rotation_layout->addWidget(animate_button, 3, 0, 1, -1);

  auto rotate_tesseract_based_on_radio_buttons = [tesseract,xy,xz,xw,yz,yw,zw](float angle){
    if (xy->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::XY);
    }
    if (xz->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::XZ);
    }
    if (xw->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::XW);
    }
    if (yz->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::YZ);
    }
    if (yw->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::YW);
    }
    if (zw->isChecked()) {
      tesseract->rotate(angle, rotation_4D::RotationPlane::ZW);
    }
  };

  connect(apply_rotation_button, &QPushButton::clicked, this,
    [rotate_tesseract_based_on_radio_buttons,angle_group](){
      rotate_tesseract_based_on_radio_buttons(angle_group->get_value());
      angle_group->setValue(0.0);
    }
  );

  // This is probably a stupid way of doing this
  std::shared_ptr<float> rotation_angle = std::make_shared<float>(0.0f);
  connect(angle_group, &Slider_Spinbox_Group::valueChanged, this,
    [rotate_tesseract_based_on_radio_buttons, rotation_angle](double a){
      float delta_angle = a-(*rotation_angle);
      *rotation_angle = a;
      rotate_tesseract_based_on_radio_buttons(delta_angle);
    }
  );

  std::shared_ptr<QMetaObject::Connection> update_tesseract_connection = std::make_shared<QMetaObject::Connection>();
  connect(animate_button, &QPushButton::clicked, this,
    [this, update_tesseract_connection, rotate_tesseract_based_on_radio_buttons, angle_group, animate_button] (bool checked) {
      auto animation = [rotate_tesseract_based_on_radio_buttons, angle_group]() {
        rotate_tesseract_based_on_radio_buttons(angle_group->get_value());
      };
      if (checked) {
        *update_tesseract_connection = connect(update_timer, &QTimer::timeout, this, animation);
        animate_button->setText("Stop Animation");
      } else {
        disconnect(*update_tesseract_connection);
        animate_button->setText("Animate");
      }
    }
  );

  connect(xy, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::XY);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::XY);
      }
    }
  );

  connect(xz, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::XZ);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::XZ);
      }
    }
  );

  connect(xw, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::XW);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::XW);
      }
    }
  );

  connect(yz, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::YZ);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::YZ);
      }
    }
  );

  connect(yw, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::YW);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::YW);
      }
    }
  );

  connect(zw, &QRadioButton::toggled, this,
    [rotate_tesseract_based_on_radio_buttons, tesseract, rotation_angle](bool checked) {
      if (!checked) {
        tesseract->rotate(-1.0f*(*rotation_angle), rotation_4D::RotationPlane::ZW);
      } else {
        tesseract->rotate(*rotation_angle, rotation_4D::RotationPlane::ZW);
      }
    }
  );

  return tesseract_box;
}

QStandardItem* Settings::set_material(Material* material) {
  QStandardItem* material_item = new QStandardItem(QString(tr(material->name.c_str())));
  if (material->textures.size() >= 1) {
    Q_ASSERT_X(material->textures[0].image.isNull() == false, "texture icons creation", "QImage is null");
    material_item->setIcon(QIcon(QPixmap::fromImage(material->textures[0].image)));
  } else {
    material_item->setIcon(icons.find("material")->second);
  }
  QVariant material_item_data;
  material_item_data.setValue(material);

  if (loaded_materials.find(material) == loaded_materials.end()) {
    // Create the window
    QWidget *Material_widget = new QWidget(this);
    QGridLayout *Material_layout = new QGridLayout(Material_widget);

    QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
    QGridLayout *Color_Layout = new QGridLayout(Color_Box);
    create_option_group("R:", &material->color.r, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 0);
    create_option_group("G:", &material->color.g, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 1);
    create_option_group("B:", &material->color.b, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 2);
    Material_layout->addWidget(Color_Box, 0, 0);

    QGroupBox *Misc_box = new QGroupBox(tr("Material Properties"), this);
    QGridLayout *Misc_layout = new QGridLayout(Misc_box);
    create_option_group("Ambient:", &material->ambient, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 0);
    create_option_group("Diffuse:", &material->diffuse, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 1);
    create_option_group("Specular:", &material->specular, 0.0, 5.0, 0.05, 2, Misc_box, Misc_layout, 2);
    create_option_group("Roughness:", &material->roughness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 3);
    create_option_group("Metalness:", &material->metalness, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 4);
    create_option_group("Opacity:", &material->opacity, 0.0, 1.0, 0.01, 2, Misc_box, Misc_layout, 5);
    Material_layout->addWidget(Misc_box, 0, 1);

    if (material->textures.size() >= 1) {
      QTabWidget *Image_container = new QTabWidget(this);
      for (auto texture : material->textures) {
        QLabel *texture_label = new QLabel(Image_container);
        QPixmap texture_image = QPixmap::fromImage(texture.image);
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

    loaded_materials[material] = Scrolling;

    // The material hasn't been loaded before so add it to the materials tab
    QPushButton* material_button = new QPushButton(materials_list);
    if (material->textures.size() >= 1)
    material_button->setIcon(QIcon(QPixmap::fromImage(material->textures[0].image)));
    material_button->setText(tr(material->name.c_str()));
    connect(material_button, &QPushButton::clicked, this,
      [Scrolling](){
        Scrolling->show();
        QApplication::setActiveWindow(Scrolling);
      }
    );
    materials_list_layout->addWidget(material_button);
  }

  material_item->setData(material_item_data);
  return material_item;
}

QScrollArea* Settings::set_animation(NodeAnimation* animation) {
  QWidget* animation_widget = new QWidget(this);
  QVBoxLayout* animation_layout = new QVBoxLayout(animation_widget);

  animation_layout->addWidget(new QLabel(tr("TPS: ")+QString::number(animation->tps)));
  animation_layout->addWidget(new QLabel(tr("Duration: ")+QString::number(animation->duration)));

  for (auto it : animation->animation_channels) {
    QScrollArea* animation_channel_menu = set_animation_channel(it.second);
    QPushButton* animation_channel_button = new QPushButton(tr(it.first.c_str()), animation_widget);
    animation_layout->addWidget(animation_channel_button);
    connect(animation_channel_button, &QPushButton::clicked, this,
      [animation_channel_menu]() {
        animation_channel_menu->show();
        QApplication::setActiveWindow(animation_channel_menu);
      }
    );
  }

  QScrollArea *scrolling = new QScrollArea(this);
  scrolling->setWindowFlags(Qt::Window);
  scrolling->setWindowTitle(animation->name.c_str());
  scrolling->setWidget(animation_widget);
  scrolling->setWidgetResizable(true);

  return scrolling;
}

QScrollArea* Settings::set_animation_channel(NodeAnimationChannel* animation_channel) {
  QWidget* animation_channel_widget = new QWidget(this);
  QGridLayout* animation_channel_layout = new QGridLayout(animation_channel_widget);

  Q_ASSERT_X(animation_channel->position_keys.size() > 0, "Animation settings", "Could not find any keys");

  QGroupBox *position_box = new QGroupBox(tr("Position"), animation_channel_widget);
  QGridLayout *position_layout = new QGridLayout(position_box);
  create_option_group("X:", &animation_channel->position_keys[0].vector.x, -50.0, 50.0, 0.5, 2, position_box, position_layout, 0);
  create_option_group("Y:", &animation_channel->position_keys[0].vector.y, -50.0, 50.0, 0.5, 2, position_box, position_layout, 1);
  create_option_group("Z:", &animation_channel->position_keys[0].vector.z, -50.0, 50.0, 0.5, 2, position_box, position_layout, 2);
  animation_channel_layout->addWidget(position_box, 0, 0);

  QGroupBox *scale_box = new QGroupBox(tr("Scale"), animation_channel_widget);
  QGridLayout *scale_layout = new QGridLayout(scale_box);
  create_option_group("X:", &animation_channel->scale_keys[0].vector.x, -50.0, 50.0, 0.5, 2, scale_box, scale_layout, 0);
  create_option_group("Y:", &animation_channel->scale_keys[0].vector.y, -50.0, 50.0, 0.5, 2, scale_box, scale_layout, 1);
  create_option_group("Z:", &animation_channel->scale_keys[0].vector.z, -50.0, 50.0, 0.5, 2, scale_box, scale_layout, 2);
  animation_channel_layout->addWidget(scale_box, 1, 0);

  QGroupBox *rotation_box = new QGroupBox(tr("Rotation"), animation_channel_widget);
  QGridLayout *rotation_layout = new QGridLayout(rotation_box);
  create_option_group("W:", &animation_channel->rotation_keys[0].quaternion.w, -1.0, 1.0, 1, 2, rotation_box, rotation_layout, 0);
  create_option_group("X:", &animation_channel->rotation_keys[0].quaternion.x, -1.0, 1.0, 1, 2, rotation_box, rotation_layout, 1);
  create_option_group("Y:", &animation_channel->rotation_keys[0].quaternion.y, -1.0, 1.0, 1, 2, rotation_box, rotation_layout, 2);
  create_option_group("Z:", &animation_channel->rotation_keys[0].quaternion.z, -1.0, 1.0, 1, 2, rotation_box, rotation_layout, 3);
  animation_channel_layout->addWidget(rotation_box, 0, 1, 1, -1);

  QScrollArea *scrolling = new QScrollArea(this);
  scrolling->setWindowFlags(Qt::Window);
  scrolling->setWindowTitle(tr(animation_channel->name.c_str()));
  scrolling->setWidget(animation_channel_widget);
  scrolling->setWidgetResizable(true);

  return scrolling;
}

void Settings::set_point_light(PointLight *point_light) {
  QWidget *Light_widget = new QWidget(this);
  QGridLayout *Light_layout = new QGridLayout(Light_widget);

  QGroupBox *Color_Box = new QGroupBox(tr("Color"), this);
  QGridLayout *Color_Layout = new QGridLayout(Color_Box);
  create_option_group("R:", &point_light->color.r, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &point_light->color.g, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 1);
  create_option_group("B:", &point_light->color.b, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 2);
  Light_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Lighting_Box = new QGroupBox(tr("Lighting"), this);
  QGridLayout *Lighting_Layout = new QGridLayout(Lighting_Box);
  create_option_group("Ambient:", &point_light->ambient, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 0);
  create_option_group("Diffuse:", &point_light->diffuse, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 1);
  create_option_group("Specular:", &point_light->specular, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 2);
  Light_layout->addWidget(Lighting_Box, 0, 1);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &point_light->position.x, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 0);
  create_option_group("Y:", &point_light->position.y, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 1);
  create_option_group("Z:", &point_light->position.z, -50.0, 50.0, 0.5, 2, Position_box, Position_layout, 2);
  Light_layout->addWidget(Position_box, 1, 0);

  QGroupBox *Samples_Box = new QGroupBox(tr("Samples"), this);
  QGridLayout *Samples_Layout = new QGridLayout(Samples_Box);
  create_option_group("Samples:", &point_light->samples, 1.0, 26.0, 1.0, 0, Samples_Box, Samples_Layout, 0);
  create_option_group("Sample Radius:", &point_light->sample_radius, 0.0, 1.0, 0.01, 2, Samples_Box, Samples_Layout, 1);
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
  create_option_group("R:", &dirlight->color.r, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 0);
  create_option_group("G:", &dirlight->color.g, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 1);
  create_option_group("B:", &dirlight->color.b, 0.0, 1.0, 0.1, 2, Color_Box, Color_Layout, 2);
  Light_layout->addWidget(Color_Box, 0, 0);

  QGroupBox *Lighting_Box = new QGroupBox(tr("Lighting"), this);
  QGridLayout *Lighting_Layout = new QGridLayout(Lighting_Box);
  create_option_group("Ambient:", &dirlight->ambient, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 0);
  create_option_group("Diffuse:", &dirlight->diffuse, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 1);
  create_option_group("Specular:", &dirlight->specular, 0.0, 25.0, 0.1, 1, Lighting_Box, Lighting_Layout, 2);
  Light_layout->addWidget(Lighting_Box, 0, 1);

  QGroupBox *Position_box = new QGroupBox(tr("Position"), this);
  QGridLayout *Position_layout = new QGridLayout(Position_box);
  create_option_group("X:", &dirlight->position.x, -100.0, 100.0, 1, 1, Position_box, Position_layout, 0);
  create_option_group("Y:", &dirlight->position.y, -100.0, 100.0, 1, 1, Position_box, Position_layout, 1);
  create_option_group("Z:", &dirlight->position.z, -100.0, 100.0, 1, 1, Position_box, Position_layout, 2);
  Light_layout->addWidget(Position_box, 1, 0);

  QGroupBox *Direction_box = new QGroupBox(tr("Direction"), this);
  QGridLayout *Direction_layout = new QGridLayout(Direction_box);
  create_option_group("X:", &dirlight->direction.x, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 0);
  create_option_group("Y:", &dirlight->direction.y, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 1);
  create_option_group("Z:", &dirlight->direction.z, -5.0, 5.0, 1, 1, Direction_box, Direction_layout, 2);
  Light_layout->addWidget(Direction_box, 1, 1);

  QGroupBox *View_box = new QGroupBox(tr("View"), this);
  QGridLayout *View_layout = new QGridLayout(View_box);

  create_option_group("X View Size:", &dirlight->x_view_size, 1.0, 50.0, 1.0, 0, View_box, View_layout, 0);
  create_option_group("Y View Size:", &dirlight->y_view_size, 1.0, 50.0, 1.0, 0, View_box, View_layout, 1);
  create_option_group("Near Plane:", &dirlight->near_plane, 0.01, 10.0, 0.1, 2, View_box, View_layout, 2);
  create_option_group("Far Plane:", &dirlight->far_plane, 0.1, 500.0, 1.0, 2, View_box, View_layout, 3);

  Light_layout->addWidget(View_box, 2, 0);

  QScrollArea *Scrolling = new QScrollArea(this);
  Scrolling->setWidget(Light_widget);
  Scrolling->setWidgetResizable(true);

  addTab(Scrolling, tr(dirlight->name.c_str()));
}

template <typename T>
QWidget* Settings::create_option_group(
  const char *name, T *option,
  double min_val, double max_val, double step, int decimals,
  QWidget *parent, QGridLayout *layout, int y_pos
) {
  T initial_value = *option;
  Slider_Spinbox_Group* group = new Slider_Spinbox_Group(min_val, max_val, step, decimals, name, parent);
  group->setValue(initial_value);
  connect(group, &Slider_Spinbox_Group::valueChanged, this,
    [option](double value){
      (*option) = (T)value;
    }
  );
  layout->addWidget(group, y_pos, 0, 1,-1);
  return group;
}
