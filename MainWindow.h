#include <QApplication>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QKeyEvent>
#include <QEvent>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <unordered_set>
#include <vector>

#include "OpenGLWindow.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent=nullptr);
  ~MainWindow();

  QGridLayout *window_layout;

  QGroupBox *status_box;
  QLabel *fps_label;

  std::unordered_set<int> keys_pressed;
  int delta_time;
  QPoint mouse_movement;

private slots:
  void mainLoop();

private:
  OpenGLWindow *GLWindow;

  QElapsedTimer *frame_time;
  std::vector<int> previous_frame_times;
  unsigned int current_pft_index;
  QTimer *timer;
  bool mouse_grabbed;
  bool first_mouse;

protected:
  void closeEvent(QCloseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void handleMouseMovement(); // I need to be sure that I handle the mouse movement *exactly* once per update
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  // void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
};
