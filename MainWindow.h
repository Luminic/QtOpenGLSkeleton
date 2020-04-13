#include <QApplication>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QKeyEvent>
#include <QEvent>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

#include <unordered_set>
#include <vector>

#include "OpenGLWindow.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent=nullptr);
  ~MainWindow();
  void create_pause_menu();

  void pause();
  void resume();

protected slots:
  void mainLoop();

protected:
  OpenGLWindow* GLWindow;
  QGridLayout* window_layout;

  QGroupBox* status_box;
  QLabel* fps_label;

  QWidget* pause_menu;
  QGridLayout* pause_layout;
  QLabel* pause_label;

  QToolButton* pause_button;
  QToolButton* settings_button;
  QToolButton* movement_button;

  int delta_time;
  QElapsedTimer frame_time;
  std::vector<int> previous_frame_times;
  unsigned int current_pft_index;
  QTimer *timer;

  QPoint previous_mouse_position; // Set to NULL if the mouse is not pressed
  QPoint mouse_down_position; // Relative position where the left mouse button is first pressed (set to NULL when mouse button is released)
  QPoint mouse_movement;
  bool first_mouse;
  bool grab_to_turn;

  std::unordered_set<int> keys_pressed;

  bool paused;

  void closeEvent(QCloseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  // void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
};
