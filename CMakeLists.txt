cmake_minimum_required(VERSION 3.14)

project(PanelFinder2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets Network)

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

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/PanelFinder2.rc")
qt_add_executable(PanelFinder2
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES} ${app_icon_resource_windows}
    newstonkamudpmulticast.h newstonkamudpmulticast.cpp
    mysplashscreen.h mysplashscreen.cpp
)

target_link_libraries(PanelFinder2
    PRIVATE Qt6::Core Qt6::Quick Qt6::Widgets Qt6::Network)

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

qt_import_qml_plugins(PanelFinder2)
qt_finalize_executable(PanelFinder2)

# Windows deployment target
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")

        add_custom_target(deploy
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:PanelFinder2>" "${DEPLOY_DIR}/"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --dir "${DEPLOY_DIR}"
                --qmldir "${CMAKE_SOURCE_DIR}"
                $<$<CONFIG:Release>:--release>
                $<$<CONFIG:Debug>:--debug>
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "${DEPLOY_DIR}/$<TARGET_FILE_NAME:PanelFinder2>"
            DEPENDS PanelFinder2
            COMMENT "Deploying PanelFinder2 for Windows..."
        )
        message(STATUS "Deploy target available: cmake --build . --target deploy")
    else()
        message(WARNING "windeployqt not found - 'deploy' target unavailable")
    endif()
endif()
