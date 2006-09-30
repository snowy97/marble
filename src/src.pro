######################################################################
# Automatically generated by qmake (2.00a) Mi Mai 11 23:22:14 2005
######################################################################

TEMPLATE = app
INCLUDEPATH += .
# Input
HEADERS += fastmath.h polygon.h \
	   katlascontrol.h katlasview.h katlasviewinputhandler.h texcolorizer.h texloader.h texmapper.h quaternion.h \
           katlasglobe.h katlastoolbox.h pntmap.h vectorcomposer.h vectormap.h clippainter.h placemarkmanager.h \
	   geopoint.h placecontainer.h placemarkstorage.h placefolder.h placemark.h searchlistview.h \
	   maptheme.h katlasmapscale.h placemarkmodel.h placemarkpainter.h tilescissor.h \
	   katlastilecreatordialog.h katlasthemeselectview.h katlaswindrose.h xmlhandler.h
SOURCES += fastmath.cpp \
	   katlascontrol.cpp \
           katlasview.cpp \
           katlasviewinputhandler.cpp \
           main.cpp \
           texcolorizer.cpp \
           texloader.cpp \
           texmapper.cpp \
           quaternion.cpp \
           katlasglobe.cpp \
           katlastoolbox.cpp \
           pntmap.cpp \
           vectorcomposer.cpp \
           vectormap.cpp \
           clippainter.cpp \
	   geopoint.cpp \
	   placemarkmanager.cpp \
	   placecontainer.cpp \
	   placemarkstorage.cpp \
	   placefolder.cpp \
	   placemark.cpp \
	   searchlistview.cpp \
	   katlasmapscale.cpp \
	   placemarkmodel.cpp \
	   placemarkpainter.cpp \
	   maptheme.cpp \
	   tilescissor.cpp \
	   katlastilecreatordialog.cpp \
	   katlasthemeselectview.cpp \
	   katlaswindrose.cpp \
	   xmlhandler.cpp

RESOURCES = marble.qrc

DESTDIR = ../bin
TARGET = marble
FORMS = katlastoolbox.ui katlastilecreatordialog.ui
OPTIONS = release
QMAKE_CXX_FLAGS_RELEASE += -O2 -msse

DEFINES -=KDEBUILD

QT += xml svg

win32 {
    OBJECTS_DIR=.objwin\
}

unix {
    OBJECTS_DIR=.objunx/
}
