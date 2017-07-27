#-------------------------------------------------
#
# Project created by QtCreator 2015-07-19T22:30:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FujimoriUIQt5
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    subroutines.cpp \
    detectionpart.cpp \
    dpdetection.cpp \
    alignmentpart.cpp \
    elastixalignment.cpp \
    manualdetectionpart.cpp

HEADERS  += mainwindow.h \
    rigid_parameter.h \
    structure.h

FORMS    += mainwindow.ui

#OpenCV settings
#for Windows
CONFIG(debug, debug|release){
    DESTDIR = debug
    LIBS += -LC:\\opencv2.4.9\\build\\x64\\vc12\\lib \
    -lopencv_calib3d249d \
    -lopencv_contrib249d \
    -lopencv_core249d \
    -lopencv_features2d249d \
    -lopencv_flann249d \
    -lopencv_gpu249d \
    -lopencv_highgui249d \
    -lopencv_imgproc249d \
    -lopencv_legacy249d \
    -lopencv_ml249d \
    -lopencv_objdetect249d \
    -lopencv_video249d \
    -lopencv_nonfree249d
    LIBS += -LC:\\openslide\\include
    -llibopenslide
}
else{
    DESTDIR = release
    LIBS += -LC:\\opencv2.4.9\\build\\x64\\vc12\\lib \
    -lopencv_calib3d249 \
    -lopencv_contrib249 \
    -lopencv_core249 \
    -lopencv_features2d249 \
    -lopencv_flann249 \
    -lopencv_gpu249 \
    -lopencv_highgui249 \
    -lopencv_imgproc249 \
    -lopencv_legacy249 \
    -lopencv_ml249 \
    -lopencv_objdetect249 \
    -lopencv_video249 \
    -lopencv_nonfree249
    LIBS += -LC:\\openslide\\include
    -llibopenslide
}


unix|win32: LIBS += -L$$PWD/../../../../../openslide/lib/ -llibopenslide

INCLUDEPATH += $$PWD/../../../../../openslide/include/openslide
DEPENDPATH += $$PWD/../../../../../openslide/include/openslide

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../opencv2.4.9/vc12/lib/ -lopencv_calib3d249
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../opencv2.4.9/vc12/lib/ -lopencv_calib3d249d
#else:unix: LIBS += -L$$PWD/../../../../../opencv2.4.9/vc12/lib/ -lopencv_calib3d249

INCLUDEPATH += $$PWD/../../../../../opencv2.4.9/build/include
DEPENDPATH += $$PWD/../../../../../opencv2.4.9/build/include
