QT += sql xml core concurrent

isEqual(QT_MAJOR_VERSION, 6): QT += core5compat

CONFIG += c++17

HEADERS += \
    QtEFProvider.h \
    Test/Models/countries.h \
    Test/Models/qtefprovidertest.h \
    databasemodel.h

SOURCES += \
    QtEFProvider.cpp \
    Test/Models/countries.cpp \
    Test/Models/qtefprovidertest.cpp

