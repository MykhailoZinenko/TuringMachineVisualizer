#include "MainWindow.h"
#include "DocumentTabManager.h"
#include "../document/Document.h"
#include "../project/Project.h"
#include "../project/ProjectManager.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QInputDialog>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentDocument(nullptr), m_currentProject(nullptr)
{
    // Create and set the central widget
    m_tabManager = new DocumentTabManager(this);
    setCentralWidget(m_tabManager);

    // Setup UI components
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    // Connect tab manager signals
    connect(m_tabManager, &DocumentTabManager::documentTabChanged,
            this, &MainWindow::onDocumentTabChanged);
    connect(m_tabManager, &DocumentTabManager::documentTabClosed,
            this, &MainWindow::onDocumentTabClosed);

    // Read settings
    readSettings();

    // Set window title
    updateWindowTitle();

    // Show ready status
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check if there are any unsaved projects
    bool hasUnsavedProjects = false;

    for (Project* project : ProjectManager::getInstance().getAllProjects()) {
        if (project->isModified()) {
            hasUnsavedProjects = true;
            break;
        }
    }

    if (hasUnsavedProjects) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("There are unsaved changes in one or more projects. Save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }

        if (result == QMessageBox::Save) {
            // Save all modified projects
            for (Project* project : ProjectManager::getInstance().getAllProjects()) {
                if (project->isModified()) {
                    ProjectManager::getInstance().saveProject(project);
                }
            }
        }
    }

    writeSettings();
    event->accept();
}

void MainWindow::createActions()
{
    // New Project action
    m_newProjectAction = new QAction(tr("&New Project"), this);
    m_newProjectAction->setShortcuts(QKeySequence::New);
    m_newProjectAction->setStatusTip(tr("Create a new Turing machine project"));
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);

    // Open Project action
    m_openProjectAction = new QAction(tr("&Open Project..."), this);
    m_openProjectAction->setShortcuts(QKeySequence::Open);
    m_openProjectAction->setStatusTip(tr("Open an existing project"));
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    // Save Project action
    m_saveProjectAction = new QAction(tr("&Save Project"), this);
    m_saveProjectAction->setShortcuts(QKeySequence::Save);
    m_saveProjectAction->setStatusTip(tr("Save the current project"));
    m_saveProjectAction->setEnabled(false); // Disabled until a project is active
    connect(m_saveProjectAction, &QAction::triggered, this, &MainWindow::saveProject);

    // Save As action
    m_saveAsProjectAction = new QAction(tr("Save Project &As..."), this);
    m_saveAsProjectAction->setShortcuts(QKeySequence::SaveAs);
    m_saveAsProjectAction->setStatusTip(tr("Save the current project with a new name"));
    m_saveAsProjectAction->setEnabled(false); // Disabled until a project is active
    connect(m_saveAsProjectAction, &QAction::triggered, this, &MainWindow::saveProjectAs);

    // Exit action
    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcuts(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus()
{
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newProjectAction);
    m_fileMenu->addAction(m_openProjectAction);
    m_fileMenu->addAction(m_saveProjectAction);
    m_fileMenu->addAction(m_saveAsProjectAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // Edit menu (placeholder)
    m_editMenu = menuBar()->addMenu(tr("&Edit"));

    // View menu (placeholder)
    m_viewMenu = menuBar()->addMenu(tr("&View"));

    // Help menu (placeholder)
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createToolbars()
{
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newProjectAction);
    m_fileToolBar->addAction(m_openProjectAction);
    m_fileToolBar->addAction(m_saveProjectAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("YourOrganization", "TuringMachineVisualizer");

    // Restore window geometry
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        // Default size
        const QRect availableGeometry = QApplication::primaryScreen()->availableGeometry();
        resize(availableGeometry.width() * 0.8, availableGeometry.height() * 0.7);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }

    // Restore window state
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::writeSettings()
{
    QSettings settings("YourOrganization", "TuringMachineVisualizer");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::newProject()
{
    // Get a project name
    bool ok;
    QString name = QInputDialog::getText(
        this,
        tr("New Turing Machine Project"),
        tr("Enter a name for the new project:"),
        QLineEdit::Normal,
        tr("Untitled"),
        &ok
    );

    if (!ok || name.isEmpty()) {
        name = tr("Untitled");
    }

    // Create a new project
    Project* project = ProjectManager::getInstance().createProject(name.toStdString());

    // Open the project in tabs
    if (project) {
        m_tabManager->openProject(project);
        statusBar()->showMessage(tr("Created new project: %1").arg(name), 2000);
    }
}

void MainWindow::openProject()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Project"),
        QString(),
        tr("Turing Machine Projects (*.tmproj);;All Files (*)")
    );

    if (filePath.isEmpty()) return;

    // Check if already open
    Project* existingProject = ProjectManager::getInstance().findProjectByPath(filePath.toStdString());
    if (existingProject) {
        // Just switch to the project's tabs
        m_tabManager->openProject(existingProject);
        statusBar()->showMessage(tr("Project already open: %1").arg(
            QString::fromStdString(existingProject->getName())), 2000);
        return;
    }

    // Try to open the project
    Project* project = ProjectManager::getInstance().openProject(filePath.toStdString());

    if (project) {
        // Open the project's documents in tabs
        m_tabManager->openProject(project);
        statusBar()->showMessage(tr("Opened project: %1").arg(
            QString::fromStdString(project->getName())), 2000);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open the project"));
    }
}

void MainWindow::saveProject()
{
    if (!m_currentProject) return;

    if (m_currentProject->getFilePath().empty()) {
        saveProjectAs();
        return;
    }

    if (ProjectManager::getInstance().saveProject(m_currentProject)) {
        statusBar()->showMessage(tr("Project saved"), 2000);
        updateWindowTitle();
    } else {
        QMessageBox::warning(
            this,
            tr("Save Error"),
            tr("Failed to save the project")
        );
    }
}

void MainWindow::saveProjectAs()
{
    if (!m_currentProject) return;

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Project As"),
        QString::fromStdString(m_currentProject->getName()),
        tr("Turing Machine Projects (*.tmproj)")
    );

    if (filePath.isEmpty()) return;

    // Add extension if missing
    if (!filePath.endsWith(".tmproj")) {
        filePath += ".tmproj";
    }

    if (ProjectManager::getInstance().saveProjectAs(m_currentProject, filePath.toStdString())) {
        statusBar()->showMessage(tr("Project saved as %1").arg(filePath), 2000);
        updateWindowTitle();
    } else {
        QMessageBox::warning(
            this,
            tr("Save Error"),
            tr("Failed to save the project as %1").arg(filePath)
        );
    }
}

void MainWindow::onDocumentTabChanged(Document* document)
{
    m_currentDocument = document;

    // Update the current project
    if (document) {
        m_currentProject = document->getProject();
    } else {
        m_currentProject = nullptr;
    }

    updateUIForDocument(document);
}

void MainWindow::onDocumentTabClosed(Document* document)
{
    // If we're closing the current document, update the UI
    if (document == m_currentDocument) {
        m_currentDocument = nullptr;
        updateUIForDocument(nullptr);
    }
}

void MainWindow::updateWindowTitle()
{
    if (m_currentProject) {
        QString title = QString::fromStdString(m_currentProject->getName());
        if (m_currentProject->isModified()) {
            title += "*";
        }
        setWindowTitle(title + " - Turing Machine Visualizer");
    } else {
        setWindowTitle("Turing Machine Visualizer");
    }
}

void MainWindow::updateUIForDocument(Document* document)
{
    updateWindowTitle();

    // Enable/disable actions based on having a document
    m_saveProjectAction->setEnabled(m_currentProject != nullptr);
    m_saveAsProjectAction->setEnabled(m_currentProject != nullptr);

    // Update status bar
    if (document) {
        statusBar()->showMessage(tr("Document: %1")
            .arg(QString::fromStdString(document->getName())));
    } else {
        statusBar()->showMessage(tr("No document selected"));
    }
}