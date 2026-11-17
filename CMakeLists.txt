cmake_minimum_required(VERSION 3.16)
project(MatrixAnimation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Try Qt6 first, fall back to Qt5
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets)
    set(QT_FOUND TRUE)
else()
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets)
endif()

add_executable(MatrixAnimation
    main.cpp
    matrix_raindrop.cpp
    matrix_widget.cpp
    main_window.cpp
)

target_link_libraries(MatrixAnimation PRIVATE ${QT_LIBS})