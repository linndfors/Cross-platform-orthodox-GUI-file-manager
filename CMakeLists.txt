cmake_minimum_required(VERSION 3.5)

project(file_manager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set the path to the directory containing Qt5Config.cmake
#set(Qt6_DIR "/home/nickolay/Qt/6.6.0/gcc_64/lib/cmake/Qt6")

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)


add_executable(file_manager
    main.cpp
    mainwidget.cpp
    mainwidget.ui
)

target_link_libraries(file_manager
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Default rules for deployment.
if(QNX)
    set(target_path /tmp/${TARGET}/bin)
else()
    if(UNIX AND NOT ANDROID)
        set(target_path /opt/${TARGET}/bin)
    endif()
endif()

if(NOT "${target_path}" STREQUAL "")
    install(TARGETS file_manager DESTINATION ${target_path})
endif()
