#include "Utility.h"

Slider_Spinbox_Group::Slider_Spinbox_Group(const QString& name, QWidget* parent) : QWidget(parent) {
  this->name = name;
  init();
}

Slider_Spinbox_Group::Slider_Spinbox_Group(double minimum, double maximum, double step, int decimals, const QString& name, QWidget* parent) : QWidget(parent) {
  this->minimum = minimum;
  this->maximum = maximum;
  this->step = step;
  this->decimals = decimals;
  this->name = name;
  init();
}

void Slider_Spinbox_Group::connect_slots_and_signals() {
  connect(slider, &QSlider::valueChanged, this,
    [this](int value){
      setValue(double(value)/conversion_factor);
    }
  );
  connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Slider_Spinbox_Group::setValue);
  connect(this, &Slider_Spinbox_Group::valueChanged, spinbox, &QDoubleSpinBox::setValue);
  connect(this, &Slider_Spinbox_Group::valueChanged, slider,
    [this](double value){
      slider->setValue(int(value*conversion_factor));
    }
  );
}

void Slider_Spinbox_Group::setValue(double value) {
  if (glm::abs(value-this->value) > 0.01) {
    emit valueChanged(value);
    this->value = value;
  }
}

void Slider_Spinbox_Group::init() {
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

Matrix_4x4_View::Matrix_4x4_View(QWidget* parent) : QGroupBox(parent) {
  init();
  set_matrix(glm::mat4(1.0f));
}

Matrix_4x4_View::Matrix_4x4_View(const QString& name, QWidget* parent) : QGroupBox(name, parent) {
  init();
  set_matrix(glm::mat4(1.0f));
}

Matrix_4x4_View::Matrix_4x4_View(const glm::mat4& matrix, QWidget* parent) : QGroupBox(parent) {
  init();
  set_matrix(matrix);
}

void Matrix_4x4_View::init() {
  layout = new QGridLayout(this);
  for (unsigned int x=0; x<4; x++) {
    for (unsigned int y=0; y<4; y++) {
      values[x][y] = new QDoubleSpinBox(this);
      values[x][y]->setDecimals(2);
      values[x][y]->setRange(-50.0,50.0);
      values[x][y]->setButtonSymbols(QAbstractSpinBox::NoButtons);
      layout->addWidget(values[x][y], y, x);
      connect(values[x][y], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
        [this, x, y](double new_value) {
          set_value_at(x, y, new_value);
        }
      );
    }
  }
}

void Matrix_4x4_View::set_matrix(const glm::mat4& matrix) {
  bool changed = false;
  for (unsigned int x=0; x<4; x++) {
    for (unsigned int y=0; y<4; y++) {
      if (glm::abs(this->matrix[x][y]-matrix[x][y]) >= 0.01) {
        values[x][y]->setValue(matrix[x][y]);
        changed = true;
      }
    }
  }
  this->matrix = matrix;
  if (changed == true) {
    emit value_changed(matrix);
  }
}

void Matrix_4x4_View::set_value_at(unsigned int x, unsigned int y, float value) {
  if (glm::abs(matrix[x][y]-value) >= 0.01) {
    matrix[x][y] = value;
    values[x][y]->setValue(value);
    emit value_changed(matrix);
  }
}
