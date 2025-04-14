#include "MainWindow.h"
#include "FileListManager.h"
#include "CodeTabManager.h"
#include "TapeTabManager.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"
#include "../core/SessionManager.h"

#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QDockWidget>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QDebug>
#include <QTextEdit>

#include "StyleKit.h"

class QTextEdit;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Create actions, menus, and toolbars
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    // Create dock widgets and central widget
    createDockWidgets();
    createCentralWidget();

    // Read settings
    readSettings();

    // Update UI
    updateWindowTitle();
    updateActions();

    // Show ready status
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    // File menu actions
    m_newCodeAction = new QAction(tr("&New Code File"), this);
    m_newCodeAction->setShortcuts(QKeySequence::New);
    m_newCodeAction->setStatusTip(tr("Create a new Turing machine code file"));
    m_newCodeAction->setIcon(QIcon::fromTheme("document-new"));
    connect(m_newCodeAction, &QAction::triggered, this, &MainWindow::newCodeFile);

    m_newTapeAction = new QAction(tr("New &Tape File"), this);
    m_newTapeAction->setStatusTip(tr("Create a new tape file"));
    m_newTapeAction->setIcon(QIcon::fromTheme("document-new"));
    connect(m_newTapeAction, &QAction::triggered, this, &MainWindow::newTapeFile);

    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcuts(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing file"));
    m_openAction->setIcon(QIcon::fromTheme("document-open"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);

    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcuts(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current file"));
    m_saveAction->setIcon(QIcon::fromTheme("document-save"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    m_saveAsAction = new QAction(tr("Save &As..."), this);
    m_saveAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the current file with a new name"));
    m_saveAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcuts(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    m_exitAction->setIcon(QIcon::fromTheme("application-exit"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::exit);

    // Edit menu actions
    m_cutAction = new QAction(tr("Cu&t"), this);
    m_cutAction->setShortcuts(QKeySequence::Cut);
    m_cutAction->setStatusTip(tr("Cut the selection to the clipboard"));
    m_cutAction->setIcon(QIcon::fromTheme("edit-cut"));
    connect(m_cutAction, &QAction::triggered, this, &MainWindow::cut);

    m_copyAction = new QAction(tr("&Copy"), this);
    m_copyAction->setShortcuts(QKeySequence::Copy);
    m_copyAction->setStatusTip(tr("Copy the selection to the clipboard"));
    m_copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copy);

    m_pasteAction = new QAction(tr("&Paste"), this);
    m_pasteAction->setShortcuts(QKeySequence::Paste);
    m_pasteAction->setStatusTip(tr("Paste from the clipboard"));
    m_pasteAction->setIcon(QIcon::fromTheme("edit-paste"));
    connect(m_pasteAction, &QAction::triggered, this, &MainWindow::paste);

    // View menu actions
    m_toggleFileListAction = new QAction(tr("&File List"), this);
    m_toggleFileListAction->setCheckable(true);
    m_toggleFileListAction->setChecked(true);
    m_toggleFileListAction->setStatusTip(tr("Toggle file list visibility"));
    connect(m_toggleFileListAction, &QAction::triggered, this, &MainWindow::toggleFileList);

    // Help menu actions
    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    m_aboutAction->setIcon(QIcon::fromTheme("help-about"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus()
{
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newCodeAction);
    m_fileMenu->addAction(m_newTapeAction);
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // Edit menu
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_cutAction);
    m_editMenu->addAction(m_copyAction);
    m_editMenu->addAction(m_pasteAction);

    // View menu
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_toggleFileListAction);

    // Help menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolbars()
{
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->setMovable(true);
    m_fileToolBar->setIconSize(QSize(22, 22));
    m_fileToolBar->addAction(m_newCodeAction);
    m_fileToolBar->addAction(m_newTapeAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);

    // Edit toolbar
    m_editToolBar = addToolBar(tr("Edit"));
    m_editToolBar->setMovable(true);
    m_editToolBar->setIconSize(QSize(22, 22));
    m_editToolBar->addAction(m_cutAction);
    m_editToolBar->addAction(m_copyAction);
    m_editToolBar->addAction(m_pasteAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));

    // Add a permanent status message for the application version
    QLabel* versionLabel = new QLabel(tr("Version 1.0"), this);
    versionLabel->setFrameStyle(QFrame::NoFrame);
    versionLabel->setStyleSheet("color: #64748B; padding-right: 8px;"); // Subtle gray
    statusBar()->addPermanentWidget(versionLabel);
}

void MainWindow::createDockWidgets()
{
    // Create file list dock widget
    m_fileListDock = new QDockWidget(tr("File List"), this);
    m_fileListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_fileListDock->setStyleSheet(StyleKit::getDockStyle());

    m_fileListManager = new FileListManager(m_fileListDock);
    m_fileListDock->setWidget(m_fileListManager);

    addDockWidget(Qt::LeftDockWidgetArea, m_fileListDock);

    // Connect file list signals
    connect(m_fileListManager, &FileListManager::codeFileSelected,
            this, &MainWindow::onCodeFileSelected);
    connect(m_fileListManager, &FileListManager::tapeFileSelected,
            this, &MainWindow::onTapeFileSelected);
    connect(m_fileListManager, &FileListManager::codeDocumentCreated,
            this, &MainWindow::onCodeDocumentCreated);
    connect(m_fileListManager, &FileListManager::tapeDocumentCreated,
            this, &MainWindow::onTapeDocumentCreated);
}

void MainWindow::createCentralWidget()
{
    // Create horizontal splitter
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // Add some style to the splitter
    m_splitter->setHandleWidth(1);
    m_splitter->setStyleSheet(
        "QSplitter::handle {"
        "  background-color: #CBD5E1;"  // Light gray
        "}"
        "QSplitter::handle:hover {"
        "  background-color: #3B82F6;"  // Blue when hovered
        "}"
    );

    // Create tab managers
    m_codeTabManager = new CodeTabManager(m_splitter);
    m_tapeTabManager = new TapeTabManager(m_splitter);

    // Apply modern styles to tab managers
    m_codeTabManager->setStyleSheet(StyleKit::getTabStyle());
    m_tapeTabManager->setStyleSheet(StyleKit::getTabStyle());

    // Add to splitter
    m_splitter->addWidget(m_codeTabManager);
    m_splitter->addWidget(m_tapeTabManager);

    // Set as central widget
    setCentralWidget(m_splitter);

    // Set initial sizes
    QList<int> sizes;
    sizes << width() / 2 << width() / 2;
    m_splitter->setSizes(sizes);

    // Connect tab manager signals
    connect(m_codeTabManager, &CodeTabManager::activeDocumentChanged,
            this, &MainWindow::onActiveCodeDocumentChanged);
    connect(m_tapeTabManager, &TapeTabManager::activeDocumentChanged,
            this, &MainWindow::onActiveTapeDocumentChanged);
    connect(m_codeTabManager, &CodeTabManager::documentClosed,
            this, &MainWindow::onCodeDocumentClosed);
    connect(m_tapeTabManager, &TapeTabManager::documentClosed,
            this, &MainWindow::onTapeDocumentClosed);
}

void MainWindow::readSettings()
{
    QSettings settings;

    // Restore window geometry
    const QByteArray geometry = settings.value("MainWindow/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        // Default size and position
        const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
        resize(availableGeometry.width() * 3/4, availableGeometry.height() * 3/4);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }

    // Restore window state
    const QByteArray state = settings.value("MainWindow/state").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // Restore file list visibility
    bool fileListVisible = settings.value("MainWindow/fileListVisible", true).toBool();
    m_fileListDock->setVisible(fileListVisible);
    m_toggleFileListAction->setChecked(fileListVisible);

    // Restore splitter sizes
    QList<int> splitterSizes = settings.value("MainWindow/splitterSizes").value<QList<int>>();
    if (!splitterSizes.isEmpty()) {
        m_splitter->setSizes(splitterSizes);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings;

    // Save window geometry and state
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());

    // Save file list visibility
    settings.setValue("MainWindow/fileListVisible", m_fileListDock->isVisible());

    // Save splitter sizes
    settings.setValue("MainWindow/splitterSizes", QVariant::fromValue(m_splitter->sizes()));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Check for unsaved changes
    if (checkUnsavedChanges()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::checkUnsavedChanges()
{
    // Check code documents
    CodeDocument* activeCodeDoc = m_codeTabManager->getActiveDocument();
    if (activeCodeDoc && activeCodeDoc->isModified()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Save Changes"),
            tr("The document '%1' has unsaved changes. Save before closing?")
                .arg(QString::fromStdString(activeCodeDoc->getName())),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (result == QMessageBox::Cancel) {
            return false;
        }

        if (result == QMessageBox::Save) {
            if (!m_codeTabManager->saveCurrentDocument()) {
                return false;
            }
        }
    }

    // Check tape documents
    TapeDocument* activeTapeDoc = m_tapeTabManager->getActiveDocument();
    if (activeTapeDoc && activeTapeDoc->isModified()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Save Changes"),
            tr("The document '%1' has unsaved changes. Save before closing?")
                .arg(QString::fromStdString(activeTapeDoc->getName())),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (result == QMessageBox::Cancel) {
            return false;
        }

        if (result == QMessageBox::Save) {
            if (!m_tapeTabManager->saveCurrentDocument()) {
                return false;
            }
        }
    }

    return true;
}

void MainWindow::updateWindowTitle()
{
    QString title = tr("Turing Machine Visualizer");

    CodeDocument* codeDoc = m_codeTabManager->getActiveDocument();
    TapeDocument* tapeDoc = m_tapeTabManager->getActiveDocument();

    if (codeDoc) {
        QString codeName = QString::fromStdString(codeDoc->getName());
        if (codeDoc->isModified()) {
            codeName += "*";
        }

        title = codeName + " - " + title;
    }

    if (tapeDoc) {
        QString tapeName = QString::fromStdString(tapeDoc->getName());
        if (tapeDoc->isModified()) {
            tapeName += "*";
        }

        statusBar()->showMessage(tr("Tape: %1").arg(tapeName));
    }

    setWindowTitle(title);
}

void MainWindow::updateActions()
{
    CodeDocument* codeDoc = m_codeTabManager->getActiveDocument();
    TapeDocument* tapeDoc = m_tapeTabManager->getActiveDocument();

    // File menu actions
    m_saveAction->setEnabled(codeDoc != nullptr || tapeDoc != nullptr);
    m_saveAsAction->setEnabled(codeDoc != nullptr || tapeDoc != nullptr);

    // Edit menu actions
    bool canEdit = (codeDoc != nullptr);
    m_cutAction->setEnabled(canEdit);
    m_copyAction->setEnabled(canEdit);
    m_pasteAction->setEnabled(canEdit);
}

void MainWindow::newCodeFile()
{
    // Create a new code document
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("New Code File"),
        QString(),
        tr("Turing Machine Files (*.tm)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    // Ensure it has .tm extension
    if (!fileName.toLower().endsWith(".tm")) {
        fileName += ".tm";
    }

    // Create and open the document
    CodeDocument* document = m_codeTabManager->createDocument(fileName.toStdString());
    if (document) {
        // Add to file list
        m_fileListManager->addCodeFile(fileName);

        // Update UI
        updateWindowTitle();
        updateActions();

        statusBar()->showMessage(tr("Created new code file '%1'").arg(fileName), 2000);
    }
}

void MainWindow::newTapeFile()
{
    // Create a new tape document
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("New Tape File"),
        QString(),
        tr("Tape Files (*.tmt)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    // Ensure it has .tmt extension
    if (!fileName.toLower().endsWith(".tmt")) {
        fileName += ".tmt";
    }

    // Create and open the document
    TapeDocument* document = m_tapeTabManager->createDocument(fileName.toStdString());
    if (document) {
        // Add to file list
        m_fileListManager->addTapeFile(fileName);

        // Update UI
        updateWindowTitle();
        updateActions();

        statusBar()->showMessage(tr("Created new tape file '%1'").arg(fileName), 2000);
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        QString(),
        tr("Turing Machine Files (*.tm);;Tape Files (*.tmt);;All Files (*)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    if (fileName.toLower().endsWith(".tm")) {
        onCodeFileSelected(fileName);
    } else if (fileName.toLower().endsWith(".tmt")) {
        onTapeFileSelected(fileName);
    } else {
        QMessageBox::warning(
            this,
            tr("Unknown File Type"),
            tr("The file '%1' has an unknown file type.")
                .arg(fileName)
        );
    }
}

void MainWindow::saveFile()
{
    // Determine which tab manager is active
    QWidget* focusWidget = QApplication::focusWidget();
    bool codeActive = false;

    while (focusWidget) {
        if (focusWidget == m_codeTabManager) {
            codeActive = true;
            break;
        } else if (focusWidget == m_tapeTabManager) {
            codeActive = false;
            break;
        }
        focusWidget = focusWidget->parentWidget();
    }

    if (codeActive) {
        if (m_codeTabManager->saveCurrentDocument()) {
            statusBar()->showMessage(tr("File saved"), 2000);
            updateWindowTitle();
        }
    } else {
        if (m_tapeTabManager->saveCurrentDocument()) {
            statusBar()->showMessage(tr("File saved"), 2000);
            updateWindowTitle();
        }
    }
}

void MainWindow::saveFileAs()
{
    // Determine which tab manager is active
    QWidget* focusWidget = QApplication::focusWidget();
    bool codeActive = false;

    while (focusWidget) {
        if (focusWidget == m_codeTabManager) {
            codeActive = true;
            break;
        } else if (focusWidget == m_tapeTabManager) {
            codeActive = false;
            break;
        }
        focusWidget = focusWidget->parentWidget();
    }

    if (codeActive) {
        if (m_codeTabManager->saveCurrentDocumentAs()) {
            CodeDocument* document = m_codeTabManager->getActiveDocument();
            if (document) {
                m_fileListManager->addCodeFile(QString::fromStdString(document->getFilePath()));
                statusBar()->showMessage(tr("File saved"), 2000);
                updateWindowTitle();
            }
        }
    } else {
        if (m_tapeTabManager->saveCurrentDocumentAs()) {
            TapeDocument* document = m_tapeTabManager->getActiveDocument();
            if (document) {
                m_fileListManager->addTapeFile(QString::fromStdString(document->getFilePath()));
                statusBar()->showMessage(tr("File saved"), 2000);
                updateWindowTitle();
            }
        }
    }
}

void MainWindow::exit()
{
    close();
}

void MainWindow::cut()
{
    // Pass to active code editor
    QWidget* focusWidget = QApplication::focusWidget();
    if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(focusWidget)) {
        textEdit->cut();
    }
}

void MainWindow::copy()
{
    // Pass to active code editor
    QWidget* focusWidget = QApplication::focusWidget();
    if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(focusWidget)) {
        textEdit->copy();
    }
}

void MainWindow::paste()
{
    // Pass to active code editor
    QWidget* focusWidget = QApplication::focusWidget();
    if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(focusWidget)) {
        textEdit->paste();
    }
}

void MainWindow::toggleFileList()
{
    m_fileListDock->setVisible(!m_fileListDock->isVisible());
    m_toggleFileListAction->setChecked(m_fileListDock->isVisible());
}

void MainWindow::about()
{
    QMessageBox::about(
        this,
        tr("About Turing Machine Visualizer"),
        tr("The <b>Turing Machine Visualizer</b> allows you to create, edit, "
           "and visualize Turing machines.<br><br>"
           "Version 1.0")
    );
}

void MainWindow::onCodeFileSelected(const QString& path)
{
    // Check if already open
    CodeDocument* document = m_codeTabManager->findDocumentByPath(path.toStdString());
    if (document) {
        // Just select it
        m_codeTabManager->openDocument(document);
    } else {
        // Create new document
        document = m_codeTabManager->createDocument(path.toStdString());
        if (document) {
            // Add to file list
            m_fileListManager->addCodeFile(path);
        }
    }

    // Update UI
    updateWindowTitle();
    updateActions();
}

void MainWindow::onTapeFileSelected(const QString& path)
{
    // Check if already open
    TapeDocument* document = m_tapeTabManager->findDocumentByPath(path.toStdString());
    if (document) {
        // Just select it
        m_tapeTabManager->openDocument(document);
    } else {
        // Create new document
        document = m_tapeTabManager->createDocument(path.toStdString());
        if (document) {
            // Add to file list
            m_fileListManager->addTapeFile(path);
        }
    }

    // Update UI
    updateWindowTitle();
    updateActions();
}

void MainWindow::onCodeDocumentCreated(CodeDocument* document)
{
    if (document) {
        m_codeTabManager->openDocument(document);

        // Update UI
        updateWindowTitle();
        updateActions();
    }
}

void MainWindow::onTapeDocumentCreated(TapeDocument* document)
{
    if (document) {
        m_tapeTabManager->openDocument(document);

        // Update UI
        updateWindowTitle();
        updateActions();
    }
}

void MainWindow::onCodeDocumentClosed(CodeDocument* document)
{
    Q_UNUSED(document);

    // Update UI
    updateWindowTitle();
    updateActions();
}

void MainWindow::onTapeDocumentClosed(TapeDocument* document)
{
    Q_UNUSED(document);

    // Update UI
    updateWindowTitle();
    updateActions();
}

void MainWindow::onActiveCodeDocumentChanged(CodeDocument* document)
{
    // Update SessionManager
    SessionManager::getInstance().setActiveCodeDocument(document);

    // Update file list selection
    if (document) {
        m_fileListManager->setActiveDocument(document);
    }

    // Update UI
    updateWindowTitle();
    updateActions();
}

void MainWindow::onActiveTapeDocumentChanged(TapeDocument* document)
{
    // Update SessionManager
    SessionManager::getInstance().setActiveTapeDocument(document);

    // Update file list selection
    if (document) {
        m_fileListManager->setActiveDocument(document);
    }

    // Update UI
    updateWindowTitle();
    updateActions();
}