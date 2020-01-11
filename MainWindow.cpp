#include <QDebug>
#include <QCursor>
#include <QString>
#include <QVBoxLayout>
#include <QFileDialog>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // Set up the window
  setWindowTitle("QtGL");
  resize(800, 600);

  GLWindow = new OpenGLWindow(this);
  window_layout = new QGridLayout(GLWindow);
  window_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  setCentralWidget(GLWindow);

  mouse_grabbed = false;
  first_mouse = true;
  GLWindow->set_inputs(&keys_pressed, &mouse_movement, &delta_time);

  // Set up the status box
  status_box = new QGroupBox(tr("Status Box"), this);
  status_box->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  QVBoxLayout *status_layout = new QVBoxLayout(status_box);
  fps_label = new QLabel(tr("Frame time:"), status_box);
  status_layout->addWidget(fps_label);
  window_layout->addWidget(status_box, 0, 0);
  status_box->hide();

  // Set up mainloop
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MainWindow::mainLoop);
  timer->start(16);
  frame_time = new QElapsedTimer();
  previous_frame_times = {16,16,16,16,16,16,16};
  current_pft_index = 0;
  frame_time->start();

  // Show the main window (also shows child widget: GLWindow)
  show();
}

MainWindow::~MainWindow() {
  delete frame_time;
  delete status_box;
}

void MainWindow::mainLoop() {
  delta_time = frame_time->elapsed();
  frame_time->start();

  if (!status_box->isHidden()) {
    previous_frame_times[++current_pft_index] = delta_time;
    if (current_pft_index>=previous_frame_times.size()) current_pft_index = 0;
    double fps = 0.0;
    for (auto i : previous_frame_times) fps += i;
    fps = previous_frame_times.size()/fps*1000.0;

    fps_label->setText(QString("Frame time:")+QString::number(delta_time)+QString("\nFPS:")+QString::number(fps)+QString("\nFOV:")+QString::number((int)GLWindow->fov));
  }

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
    case Qt::Key_F2:{
      QFileDialog dialog(this);
      dialog.setWindowTitle("Save Screenshot");
      dialog.setFileMode(QFileDialog::AnyFile);
      dialog.setAcceptMode(QFileDialog::AcceptSave);
      dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
      if (dialog.exec()) {
        QStringList fileNames(dialog.selectedFiles());
        if (QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0))) {
          GLWindow->grabFramebuffer().save(fileNames.at(0));
        } else {
          qDebug() << "Save error: bad format or filename.";
        }
      }
      break;}
    case Qt::Key_F3:
      if (status_box->isHidden())
        status_box->show();
      else
        status_box->hide();
      break;
    case Qt::Key_F4: {
      QPoint screen_top_left(geometry().left(), geometry().top());
      QColor col = GLWindow->grabFramebuffer().pixelColor(QCursor::pos()-screen_top_left);
      qDebug() << col.red()/255.0 << col.green()/255.0 << col.blue()/255.0;
      break;}
    default:
      keys_pressed.insert(event->key());
      break;
  }
  event->accept();
}

// void MainWindow::wheelEvent(QWheelEvent *event) {
  // qDebug() << "Pixel Delta:" << event->pixelDelta();
  // qDebug() << "Angle Delta:" << event->angleDelta();
  // event->accept();
// }

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
  keys_pressed.erase(event->key());
  event->accept();
}
