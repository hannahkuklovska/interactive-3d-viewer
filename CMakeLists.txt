cmake_minimum_required(VERSION 3.16)

# Set the project name and required C++ version
project(ImageViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automatic features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define Qt version
set(Qt6_Version 6.8.0)

# Ensure the correct Qt paths for macOS
set(Qt6_DIR "/Users/$ENV{USER}/Qt/${Qt6_Version}/macos/lib/cmake/Qt6")

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

# Collect source files
file(GLOB UI_FILES src/*.ui)
file(GLOB H_FILES src/*.h)
file(GLOB CPP_FILES src/*.cpp)
file(GLOB QRC_FILES src/*.qrc)

set(SOURCE_LIST ${CPP_FILES} ${UI_FILES} ${H_FILES} ${QRC_FILES})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_LIST}
    src/Shape.h)

# Link with Qt6 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

# Organize source files in Qt Creator
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

# macOS-specific bundle configuration (optional)
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.imageviewer"
        MACOSX_BUNDLE_BUNDLE_NAME "ImageViewer"
    )
endif()
