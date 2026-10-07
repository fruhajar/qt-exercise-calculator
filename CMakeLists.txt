cmake_minimum_required(VERSION 3.16)

project(Calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

set(PROJECT_SOURCES
    main.cpp

    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mainwindow_inits.cpp
    mainwindow_handlers.cpp

    expressionevaluator.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Calculator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    qt_finalize_executable(Calculator)
endif()

target_link_libraries(Calculator PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
)

include(GNUInstallDirs)
install(TARGETS Calculator
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "calculator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple calculator")
set(CPACK_PACKAGE_DESCRIPTION "A Qt-based simple calculator with expression evaluation")

set(CPACK_DEBIAN_PACKAGE_NAME "calculator")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jaroslav Fruehauf")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6, libqt6core6, libqt6gui6")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
endif()

include(CPack)
