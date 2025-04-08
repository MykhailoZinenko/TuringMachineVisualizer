#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    QCoreApplication::setOrganizationName("YourOrganization");
    QCoreApplication::setApplicationName("Turing Machine Visualizer");
    QCoreApplication::setApplicationVersion("0.1");

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Turing Machine Visualizer");
    mainWindow.resize(1024, 768);
    mainWindow.show();

    return app.exec();
}