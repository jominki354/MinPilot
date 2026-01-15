cmake_minimum_required(VERSION 3.16)
project(minpilot_ui_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Gui 
    OpenGL
    Multimedia
    MultimediaWidgets
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/mock
)

# Mock backend sources
set(MOCK_SOURCES
    mock/mock_messaging.cpp
    mock/mock_ui_state.cpp
    element_position.cpp
)

# Main application sources
set(APP_SOURCES
    main.cpp
    control_panel.cpp
    hud_display.cpp
)

# Build executable
add_executable(ui_simulator
    ${MOCK_SOURCES}
    ${APP_SOURCES}
)

# Link libraries
target_link_libraries(ui_simulator
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::OpenGL
    Qt5::Multimedia
    Qt5::MultimediaWidgets
)

# Install
install(TARGETS ui_simulator DESTINATION bin)
