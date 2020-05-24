#ifndef UTILITY_H
#define UTILITY_H

#include <QWidget>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QString>
#include <QGridLayout>

#include <glm/glm.hpp>

class Slider_Spinbox_Group : public QWidget {
  Q_OBJECT;

public:
  Slider_Spinbox_Group(const QString& name, QWidget* parent=nullptr);
  Slider_Spinbox_Group(double minimum, double maximum, double step, int decimals, const QString& name, QWidget* parent=nullptr);
  ~Slider_Spinbox_Group() {};

  void connect_slots_and_signals();

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
  void setValue(double value);

signals:
  void valueChanged(double value);

protected:
  void init();

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

class Matrix_4x4_View : public QGroupBox {
  Q_OBJECT;

public:
  Matrix_4x4_View(QWidget* parent=nullptr);
  Matrix_4x4_View(const QString& name, QWidget* parent=nullptr);
  Matrix_4x4_View(const glm::mat4& matrix, QWidget* parent=nullptr);
  ~Matrix_4x4_View(){}


public slots:
  void set_matrix(const glm::mat4& matrix);
  void set_value_at(unsigned int x, unsigned int y, float value);

signals:
  void value_changed(glm::mat4 value);

protected:
  void init();

  glm::mat4 matrix;
  QGridLayout* layout;
  QDoubleSpinBox* values[4][4];
};

#endif
