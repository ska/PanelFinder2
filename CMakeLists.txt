cmake_minimum_required(VERSION 3.14)

project(PanelFinder2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Widgets)

add_definitions(-DUSE_SSL)

set(PROJECT_SOURCES
    main.cpp
    common.h
    panellistmodel.cpp panellistmodel.h
    systemtray.cpp systemtray.h
    udpfinder.cpp udpfinder.h
    runguard.cpp runguard.h
    serviceudp.cpp serviceudp.h
    qml.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)

    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/PanelFinder2.rc")
    qt_add_executable(PanelFinder2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES} ${app_icon_resource_windows}
        newstonkamudpmulticast.h newstonkamudpmulticast.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PanelFinder2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PanelFinder2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PanelFinder2
          ${PROJECT_SOURCES}
          serviceudp.h serviceudp.cpp
        )
    endif()
endif()

target_link_libraries(PanelFinder2
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick Qt::Widgets)

set_target_properties(PanelFinder2 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

message(STATUS "Build ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")

#target_compile_definitions(PanelFinder2 PRIVATE QT_DEBUG_NO_OUTPUT)
string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if (build_type STREQUAL release)
    target_compile_definitions(${PROJECT_NAME} PUBLIC QT_NO_DEBUG_OUTPUT)
endif()

install(TARGETS PanelFinder2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(PanelFinder2)
    qt_finalize_executable(PanelFinder2)
endif()


