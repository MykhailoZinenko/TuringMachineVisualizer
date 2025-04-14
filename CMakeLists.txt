cmake_minimum_required(VERSION 3.16)
project(TuringMachineVisualizer VERSION 1.0 LANGUAGES CXX)

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

        # Core components
        src/core/SessionManager.cpp
        src/core/FileWatcher.cpp

        # Document classes
        src/document/Document.cpp
        src/document/CodeDocument.cpp
        src/document/TapeDocument.cpp

        # Model classes
        src/model/State.cpp
        src/model/Tape.cpp
        src/model/Transition.cpp
        src/model/TuringMachine.cpp

        # Parser
        src/parser/CodeParser.cpp

        # UI - Main components
        src/ui/MainWindow.cpp
        src/ui/FileListManager.cpp
        src/ui/CodeTabManager.cpp
        src/ui/TapeTabManager.cpp

        # UI - Views
        src/ui/CodeEditorView.cpp
        src/ui/TuringMachineSyntaxHighlighter.cpp
        src/ui/TapeVisualizationView.cpp

        # UI - Tape & Properties
        src/ui/TapeWidget.cpp
        src/ui/TapeControlWidget.cpp
)

# Set header files
set(HEADERS
        # Core components
        src/core/SessionManager.h
        src/core/FileWatcher.h

        # Document classes
        src/document/Document.h
        src/document/CodeDocument.h
        src/document/TapeDocument.h

        # Model classes
        src/model/State.h
        src/model/Tape.h
        src/model/Transition.h
        src/model/TuringMachine.h

        # Parser
        src/parser/CodeParser.h

        # UI - Main components
        src/ui/MainWindow.h
        src/ui/FileListManager.h
        src/ui/CodeTabManager.h
        src/ui/TapeTabManager.h

        # UI - Views
        src/ui/CodeEditorView.h
        src/ui/TuringMachineSyntaxHighlighter.h
        src/ui/TapeVisualizationView.h

        # UI - Tape & Properties
        src/ui/TapeWidget.h
        src/ui/TapeControlWidget.h
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.turingvisualizer.app"
            MACOSX_BUNDLE_BUNDLE_NAME "Turing Machine Visualizer"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()