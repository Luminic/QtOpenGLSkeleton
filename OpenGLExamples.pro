######################################################################
# Automatically generated by qmake (3.1) Fri Nov 15 11:10:45 2019
######################################################################

TEMPLATE = app
TARGET = OpenGLExamples

QT += core gui widgets

INCLUDEPATH += .

DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += MainWindow.h OpenGLWindow.h Shader.h Camera.h Settings.h Node.h PointLight.h Model.h Mesh.h Scene.h Material.h
SOURCES += main.cpp MainWindow.cpp OpenGLWindow.cpp Shader.cpp Camera.cpp Settings.cpp Node.cpp PointLight.cpp Model.cpp Mesh.cpp Scene.cpp Material.cpp
LIBS += -lassimp