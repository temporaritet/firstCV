#-------------------------------------------------
#
# Project created by QtCreator 2016-10-18T22:42:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = firstCV
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

QMAKE_CXXFLAGS_RELEASE *= -O3

SOURCES += main.cpp\
        mainwindow.cpp \
    webcam.cpp \
    process.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    webcam.h \
    process.h \
    server.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5/
INCLUDEPATH += `pkg-config --cflags opencv`

LIBS += `pkg-config --libs opencv`
#INCLUDEPATH += /usr/include/opencv2\
#                -I/usr/local/include

#LIBS += -lopencv_core\
#        -lopencv_imgproc\
#        -lopencv_highgui\
#        -lopencv_ml\
#        -lopencv_video\
#        -lopencv_features2d\
#        -lopencv_calib3d\
#        -lopencv_objdetect\
#        -lopencv_contrib\
#        -lopencv_legacy\
#        -lopencv_flann\

RESOURCES += \
    resource.qrc
