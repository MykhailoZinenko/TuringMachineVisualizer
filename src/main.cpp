#include <QApplication>
#include "ui/MainWindow.h"
#include "ui/StyleKit.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application information
    app.setOrganizationName("TuringMachineVisualizer");
    app.setApplicationName("Turing Machine Visualizer");
    app.setApplicationVersion("1.0");

    // Apply IDE-like styling (use StyleKit::DARK for dark mode)
    StyleKit::applyToApplication(StyleKit::LIGHT);

    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}