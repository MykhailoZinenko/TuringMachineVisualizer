cmake_minimum_required(VERSION 3.16)
project(TuringMachineVisualizer VERSION 0.2 LANGUAGES CXX)

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
        # Main
        src/main.cpp

        # Project
        src/project/Project.cpp
        src/project/ProjectManager.cpp

        # Document
        src/document/Document.cpp
        src/document/CodeDocument.cpp
        src/document/TapeDocument.cpp

        # Parser
        src/parser/CodeParser.cpp

        # UI - Main components
        src/ui/MainWindow.cpp
        src/ui/DocumentTabManager.cpp

        # UI - Document Views
        src/ui/document/DocumentView.cpp
        src/ui/document/CodeEditorView.cpp
        src/ui/document/TapeVisualizationView.cpp

        # Existing UI components
        src/ui/TapeWidget.cpp

        # Model
        src/model/Tape.cpp
        src/model/State.cpp
        src/model/Transition.cpp
        src/model/TuringMachine.cpp
)

# Set header files
set(HEADERS
        # Project
        src/project/Project.h
        src/project/ProjectManager.h

        # Document
        src/document/Document.h
        src/document/CodeDocument.h
        src/document/TapeDocument.h

        # Parser
        src/parser/CodeParser.h

        # UI - Main components
        src/ui/MainWindow.h
        src/ui/DocumentTabManager.h

        # UI - Document Views
        src/ui/document/DocumentView.h
        src/ui/document/CodeEditorView.h
        src/ui/document/TapeVisualizationView.h

        # Existing UI components
        src/ui/TapeWidget.h

        # Model
        src/model/Tape.h
        src/model/State.h
        src/model/Transition.h
        src/model/TuringMachine.h
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

# Find and link the nlohmann_json library if used in TuringMachine.cpp
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(TuringMachineVisualizer PRIVATE nlohmann_json::nlohmann_json)
else()
    message(STATUS "nlohmann_json not found as a package, including as a header-only library")
    # Add header-only support for nlohmann_json
    target_include_directories(TuringMachineVisualizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
endif()

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