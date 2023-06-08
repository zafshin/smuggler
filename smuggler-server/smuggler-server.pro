QT -= gui
QT += network
QT += core
CONFIG += c++17 console
QMAKE_CXXFLAGS_RELEASE -= -O3
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
SOURCES += \
        main.cpp \
        portforwarder.cpp \
        smugglerserver.cpp
# Default rules for deployment.
HEADERS += \
    portforwarder.h \
    smugglerserver.h
