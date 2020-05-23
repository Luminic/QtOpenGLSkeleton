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
  paused = true;

  GLWindow = new OpenGLWindow(this);
  GLWindow->set_inputs(&keys_pressed, &mouse_movement, &delta_time);
  // window_layout = new QGridLayout(this);
  // window_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  setCentralWidget(GLWindow);

  first_mouse = true;

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
  previous_frame_times = {16,16,16,16,16,16,16};
  current_pft_index = 0;
  frame_time.start();

  // Show the main window (also shows child widget: GLWindow)
  show();
}

MainWindow::~MainWindow() {
  delete status_box;
}

void MainWindow::create_pause_menu() {
  pause_menu = new QWidget(this);
  pause_menu->setStyleSheet("background-color: rgba(0,0,0,75);");
  pause_menu->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  pause_layout = new QGridLayout(pause_menu);
  pause_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  pause_button = new QToolButton(pause_menu);
  pause_button->setStyleSheet("background-color: rgba(0,0,0,0); border: none;");
  pause_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  pause_button->setFixedSize(30,30);
  pause_button->setIconSize(QSize(30,30));
  pause_button->setIcon(QIcon("assets/textures/icons/pause.png"));

  pause_layout->addWidget(pause_button, 0, 0, 1, 1);
  connect(pause_button, &QPushButton::clicked, this, [this](){this->resume();});

  settings_button = new QToolButton(pause_menu);
  settings_button->setStyleSheet("background-color: rgba(0,0,0,0); border: none;");
  settings_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  settings_button->setFixedSize(30,30);
  settings_button->setIconSize(QSize(30,30));
  settings_button->setIcon(QIcon("assets/textures/icons/settings.png"));

  pause_layout->addWidget(settings_button, 0, 1, 1, 1);
  connect(settings_button, &QPushButton::clicked, this,
    [this](){
      if (this->GLWindow->settings != nullptr) {
        this->GLWindow->settings->show();
      }
    }
  );

  movement_button = new QToolButton(pause_menu);
  movement_button->setStyleSheet("background-color: rgba(0,0,0,0); border: none;");
  movement_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  movement_button->setFixedSize(30,30);
  movement_button->setIconSize(QSize(30,30));
  QIcon movement_icon;
  movement_icon.addPixmap(QPixmap("assets/textures/icons/movement.png"), QIcon::Normal, QIcon::Off);
  movement_icon.addPixmap(QPixmap("assets/textures/icons/movement_enabled.png"), QIcon::Normal, QIcon::On);
  movement_button->setIcon(movement_icon);
  movement_button->setCheckable(true);

  pause_layout->addWidget(movement_button, 0, 2, 1, 1);
  connect(movement_button, &QPushButton::toggled, this,
    [this](bool checked){
      grab_to_turn = checked;
      if (checked) {
        QCursor cursor(Qt::SizeAllCursor);
        QApplication::setOverrideCursor(cursor);
      } else {
        QApplication::restoreOverrideCursor();
      }
    }
  );

  pause_menu->show();
}

void MainWindow::pause() {
  releaseKeyboard();
  releaseMouse();
  setMouseTracking(false);
  paused = true;
  movement_button->setChecked(false);
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
  delta_time = frame_time.elapsed();
  frame_time.start();

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
  if (!paused) {
    QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
    QCursor::setPos(screen_center);
  } else {
    pause_menu->setGeometry(QRect(QPoint(5,5),pause_menu->minimumSizeHint()));
  }
  GLWindow->update_scene();
  mouse_movement = QPoint(0,0);
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

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
  if (!paused) {
    // The cursor will be moved to the screen center in the main loop
    QPoint screen_center(geometry().left()+geometry().width()/2, geometry().top()+geometry().height()/2);
    if (first_mouse) {
      first_mouse = false;
      mouse_movement = QPoint(0,0);
    } else {
      mouse_movement = QCursor::pos()-screen_center;
    }
  } else if (grab_to_turn) {
    if (!previous_mouse_position.isNull()) {
      mouse_movement += (event->pos() - previous_mouse_position) * 2;
    } else {
      mouse_movement += (event->pos() - mouse_down_position) * 2;
    }
    previous_mouse_position = event->pos();
  } else {
    mouse_movement = QPoint(0,0);
  }
  event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
  mouse_down_position = event->pos();
  event->accept();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  mouse_down_position = QPoint(0,0);
  previous_mouse_position = QPoint(0,0);
  event->accept();
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
