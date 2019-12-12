#include <QDebug>
#include <QCursor>

#include "MainWindow.h"

/*
Let me briefly describe what each option does so you'll understand :)
Include Normals: makes sure the normal vectors are included within the wavefront file (used for lighting).
Include UVs: makes sure the texture coordinates are included if any (UV = texture coordinate).
Write Materials: also exports a .mtl file that contains all the textures used and/or colors of the model.
Triangulate Faces: this option makes sure that all shapes are first transformed to triangles before exporting. This is important, because Blender by default uses quads as their primitive shapes and OpenGL prefers triangles as its input. By enabling this option, Blender automatically transforms all their shapes to triangles first so you can directly draw the model (then you don't have to take this into account when importing your model).
*/

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
  GLWindow(new OpenGLWindow(this)),
  frame_time(new QTime()),
  timer(new QTimer(this)),
  delta_time(0),
  mouse_grabbed(false),
  first_mouse(true),
  mouse_movement()
{
  setWindowTitle("QtGL");
  resize(800, 600);
  setCentralWidget(GLWindow);

  GLWindow->scene->camera->initialize_camera(&keys_pressed, &mouse_movement, &delta_time);

  // Set up mainloop
  connect(timer, &QTimer::timeout, this, &MainWindow::mainLoop);
  timer->start(16);

  // Start timer for each frame
  frame_time->start();

  // Show the main window (also shows child widget: GLWindow)
  show();
}

MainWindow::~MainWindow() {
  delete GLWindow;
}

void MainWindow::mainLoop() {
  delta_time = frame_time->elapsed();
  frame_time->start();
  //qDebug() << delta_time;
  handleMouseMovement();
  GLWindow->update_scene();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QApplication::quit();
  event->accept();
}

void MainWindow::leaveEvent(QEvent *event) {
  if (mouse_grabbed) event->ignore();
  else {
    event->accept();
    QMainWindow::leaveEvent(event);
  }
}

void MainWindow::handleMouseMovement() {
  // This function should only be called once per update
  // that is why I'm using my own class instead of the default mouseMoveEvent
  if (mouse_grabbed) {
    QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
    if (first_mouse) {
      first_mouse = false;
      mouse_movement = QPoint(0,0);
    } else {
      mouse_movement = QCursor::pos()-screen_center;
    }
    QCursor::setPos(screen_center);
  } else {
    mouse_movement = QPoint(0,0);
  }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  event->accept(); // I'm handling the mouse movement myself... just in another function
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  event->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
    case Qt::Key_Home:
      QApplication::quit();
      break;
    case Qt::Key_Escape:
      if (!mouse_grabbed) {
        grabMouse();
        setMouseTracking(true);
        mouse_grabbed = true;
        first_mouse = true; // For when the mouse is
        QCursor cursor(Qt::BlankCursor);
        QApplication::setOverrideCursor(cursor);
      } else {
        releaseMouse();
        setMouseTracking(false);
        mouse_grabbed = false;
        QCursor cursor(Qt::ArrowCursor);
        QApplication::restoreOverrideCursor();
      }
      break;
    default:
      keys_pressed.insert(event->key());
      break;
  }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
  keys_pressed.erase(event->key());
}
