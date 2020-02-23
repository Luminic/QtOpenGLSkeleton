#include <QDebug>
#include <QCursor>
#include <QString>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPushButton>

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // Set up the window
  setWindowTitle("QtGL");
  resize(800, 600);
  paused = true;

  GLWindow = new OpenGLWindow(this);
  // window_layout = new QGridLayout(this);
  // window_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  setCentralWidget(GLWindow);

  first_mouse = true;
  GLWindow->set_inputs(&keys_pressed, &mouse_movement, &delta_time);

  // Set up the status box
  status_box = new QGroupBox(tr("Status Box"), this);
  // status_box->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  QVBoxLayout *status_layout = new QVBoxLayout(status_box);
  status_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  fps_label = new QLabel(tr("Frame time:"), status_box);
  status_layout->addWidget(fps_label);
  // window_layout->addWidget(status_box, 0, 0);
  status_box->setGeometry(QRect(QPoint(10,10),status_box->minimumSizeHint()));
  status_box->hide();

  create_pause_menu();

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

void MainWindow::create_pause_menu() {
  pause_menu = new QGroupBox(this);
  // pause_menu->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  // pause_menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QGridLayout *pause_layout = new QGridLayout(pause_menu);
  pause_layout->setAlignment(Qt::AlignCenter | Qt::AlignCenter);

  pause_label = new QLabel(tr("Paused"), pause_menu);
  pause_label->setAlignment(Qt::AlignCenter | Qt::AlignCenter);
  pause_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  pause_label->setFrameStyle(QFrame::Panel | QFrame::Raised);
  pause_label->setLineWidth(3);

  QFont serifFont("Arial", 20, QFont::Bold);
  pause_label->setFont(serifFont);
  pause_label->setStyleSheet("QLabel { background-color : grey; color : black; }");

  pause_layout->addWidget(pause_label, 0, 0, 1, -1);


  // |       Resume      |
  // | Options |  Stats  |
  // |        Quit       |

  QFont button_font("Arial", 16);

  QPushButton *resume_button = new QPushButton(tr("Resume"), pause_menu);
  resume_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  resume_button->setFont(button_font);
  connect(resume_button, &QPushButton::clicked, this, [=](){resume();});
  pause_layout->addWidget(resume_button, 1, 0, 1, -1);

  QPushButton *options_button = new QPushButton(tr("Options"), pause_menu);
  options_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  options_button->setFont(button_font);
  pause_layout->addWidget(options_button, 2, 0, 1, 1);

  QPushButton *stats_button = new QPushButton(tr("Statistics"), pause_menu);
  stats_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  stats_button->setFont(button_font);
  pause_layout->addWidget(stats_button, 2, 1, 1, 1);

  QPushButton *quit_button = new QPushButton(tr("Quit"), pause_menu);
  quit_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  quit_button->setFont(button_font);
  connect(quit_button, &QPushButton::clicked, this, [](){QApplication::quit();});
  pause_layout->addWidget(quit_button, 3, 0, 1, -1);



  pause_menu->show();
}

void MainWindow::pause() {
  releaseKeyboard();
  releaseMouse();
  setMouseTracking(false);
  paused = true;
  QCursor cursor(Qt::ArrowCursor);
  QApplication::restoreOverrideCursor();

  pause_menu->show();
}

void MainWindow::resume() {
  grabKeyboard();
  grabMouse();
  setMouseTracking(true);
  paused = false;
  first_mouse = true;
  QCursor cursor(Qt::BlankCursor);
  QApplication::setOverrideCursor(cursor);

  pause_menu->hide();
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
    status_box->setGeometry(QRect(QPoint(10,10),status_box->minimumSizeHint()));
  }

  //qDebug() << delta_time;
  handleMouseMovement();
  if (!paused) {
    GLWindow->update_scene();
  } else {// We don't want to update the scene but we want still want to draw the screen properly
    GLWindow->update();
    pause_menu->setGeometry(QRect(QPoint(150,150),size()-QSize(300,300)));
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QApplication::quit();
  event->accept();
}

void MainWindow::leaveEvent(QEvent *event) {
  if (!paused) event->ignore();
  else {
    event->accept();
    QMainWindow::leaveEvent(event);
  }
}

void MainWindow::handleMouseMovement() {
  // This function should only be called once per update
  // that is why I'm using my own class instead of the default mouseMoveEvent
  if (!paused) {
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
      if (paused) {
        resume();
      } else {
        pause();
      }
      break;
    case Qt::Key_F2:{
      if (!paused)
        pause();
        
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



void MainWindow::keyReleaseEvent(QKeyEvent *event) {
  keys_pressed.erase(event->key());
  event->accept();
}
