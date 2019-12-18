#include <QApplication>
#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QEvent>
#include <unordered_set>

#include "OpenGLWindow.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent=nullptr);
  ~MainWindow();

  std::unordered_set<int> keys_pressed;
  int delta_time;
  QPoint mouse_movement;

private slots:
  void mainLoop();

private:
  OpenGLWindow *GLWindow;

  QTime *frame_time;
  QTimer *timer;
  bool mouse_grabbed;
  bool first_mouse;

protected:
  void closeEvent(QCloseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void handleMouseMovement(); // I need to be sure that I handle the mouse movement *exactly* once per update
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
};