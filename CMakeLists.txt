cmake_minimum_required(VERSION 3.16)
project(TuringMachineVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()

# Set source files
set(SOURCES
        src/main.cpp
        src/ui/MainWindow.cpp
        src/ui/StateDialog.cpp
        src/ui/TransitionDialog.cpp
        src/ui/StatesListWidget.cpp
        src/ui/TransitionsListWidget.cpp
        src/ui/TapeWidget.cpp
        src/ui/TapeControlWidget.cpp
        src/model/Tape.cpp
        src/model/State.cpp
        src/model/Transition.cpp
        src/model/TuringMachine.cpp
        src/ui/PreferencesDialog.cpp
        src/ui/PropertiesEditorWidget.cpp
)

# Set header files
set(HEADERS
        src/ui/MainWindow.h
        src/ui/StateDialog.h
        src/ui/TransitionDialog.h
        src/ui/StatesListWidget.h
        src/ui/TransitionsListWidget.h
        src/ui/TapeWidget.h
        src/ui/TapeControlWidget.h
        src/model/Tape.h
        src/model/State.h
        src/model/Transition.h
        src/model/TuringMachine.h
        src/ui/PreferencesDialog.h
        src/ui/PropertiesEditorWidget.h
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Create resources file if needed
set(RESOURCES
        # resources.qrc
)

# Create executable
add_executable(TuringMachineVisualizer ${SOURCES} ${HEADERS} ${RESOURCES})

# Link libraries
target_link_libraries(TuringMachineVisualizer PRIVATE
        Qt::Core
        Qt::Widgets
)

# Install directives
install(TARGETS TuringMachineVisualizer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# For macOS bundle
if(APPLE)
    set_target_properties(TuringMachineVisualizer PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.turingmachinevisualizer"
            MACOSX_BUNDLE_BUNDLE_NAME "Turing Machine Visualizer"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
endif()