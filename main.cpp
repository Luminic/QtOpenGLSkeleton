#include <QApplication>
//#include <QMainWindow>
#include <QSurfaceFormat>
//#include <QDebug>
#include "MainWindow.h"
//#include "OpenGLWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(4, 2);
  QSurfaceFormat::setDefaultFormat(format);

  MainWindow window;

  return app.exec();
}
