#-------------------------------------------------
#
# Project created by QtCreator 2015-03-09T00:18:34
#
#-------------------------------------------------

QT       += core gui webkitwidgets script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = postmonster
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    common.cpp \
    models/requestsmodel.cpp \
    widgets/urllineedit.cpp \
    widgets/recordform.cpp \
    widgets/editform.cpp \
    widgets/debugform.cpp \
    diagram/diagramscene.cpp \
    diagram/diagramitem.cpp \
    diagram/arrow.cpp \
    diagram/startitem.cpp \
    pluginsdialog.cpp \
    diagram/taskitem.cpp \
    networksniffer.cpp \
    workengine.cpp \
    widgets/requestsform.cpp \
    models/jsontreeitem.cpp \
    models/environmentmodel.cpp \
    widgets/checkboxheader.cpp \
    widgets/checkboxdelegate.cpp \
    models/requestsproxymodel.cpp \
    widgets/usedrequestsform.cpp \
    widgets/requeststable.cpp

HEADERS  += mainwindow.h \
    common.h \
    models/requestsmodel.h \
    widgets/urllineedit.h \
    widgets/recordform.h \
    widgets/editform.h \
    widgets/debugform.h \
    diagram/diagramscene.h \
    diagram/diagramitem.h \
    diagram/arrow.h \
    diagram/startitem.h \
    pluginsdialog.h \
    diagram/taskitem.h \
    networksniffer.h \
    public/juicyposter.h \
    workengine.h \
    widgets/requestsform.h \
    models/jsontreeitem.h \
    models/environmentmodel.h \
    widgets/checkboxheader.h \
    widgets/checkboxdelegate.h \
    models/requestsproxymodel.h \
    widgets/usedrequestsform.h \
    widgets/requeststable.h

FORMS    += mainwindow.ui \
    widgets/recordform.ui \
    widgets/editform.ui \
    widgets/debugform.ui \
    pluginsdialog.ui \
    widgets/requestsform.ui

RESOURCES += \
    postmonster.qrc


LIBS += -L$$PWD/tools -lhttptool
