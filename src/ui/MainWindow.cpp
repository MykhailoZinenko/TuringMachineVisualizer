#include "MainWindow.h"
#include "DocumentTabManager.h"
#include "../document/Document.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"
#include "../document/DocumentManager.h"
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
    : QMainWindow(parent), m_currentDocument(nullptr)
{
    // Create and set the central widget
    m_tabManager = new DocumentTabManager(this);
    setCentralWidget(m_tabManager);

    // Setup UI components
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();

    // Read settings
    readSettings();

    // Connect document manager signals
    connect(&DocumentManager::getInstance(), &DocumentManager::documentAdded,
            this, &MainWindow::onDocumentAdded);

    // Connect tab manager signals
    connect(m_tabManager, &DocumentTabManager::documentTabChanged,
            this, &MainWindow::onDocumentTabChanged);
    connect(m_tabManager, &DocumentTabManager::documentTabClosed,
            this, &MainWindow::onDocumentTabClosed);

    // Set window title
    setWindowTitle("Turing Machine Visualizer");

    // Show ready status
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check for any modified documents
    bool hasModifiedDocuments = false;
    for (Document* doc : DocumentManager::getInstance().getAllDocuments()) {
        if (doc->isModified()) {
            hasModifiedDocuments = true;
            break;
        }
    }

    if (hasModifiedDocuments) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("There are unsaved changes. Do you want to save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (result == QMessageBox::Cancel) {
            event->ignore();
            return;
        }

        if (result == QMessageBox::Save) {
            // Save all modified documents
            for (Document* doc : DocumentManager::getInstance().getAllDocuments()) {
                if (doc->isModified()) {
                    DocumentManager::getInstance().saveDocument(doc);
                }
            }
        }
    }

    writeSettings();
    event->accept();
}

void MainWindow::createActions()
{
    // New Code Document action
    m_newCodeAction = new QAction(tr("&New Code"), this);
    m_newCodeAction->setShortcuts(QKeySequence::New);
    m_newCodeAction->setStatusTip(tr("Create a new Turing machine"));
    connect(m_newCodeAction, &QAction::triggered, this, &MainWindow::newCodeDocument);

    // Open Document action
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcuts(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing document"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openDocument);

    // Save Document action
    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcuts(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current document"));
    m_saveAction->setEnabled(false); // Disabled until a document is active
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveDocument);

    // Save As action
    m_saveAsAction = new QAction(tr("Save &As..."), this);
    m_saveAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the current document with a new name"));
    m_saveAsAction->setEnabled(false); // Disabled until a document is active
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveDocumentAs);

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
    m_fileMenu->addAction(m_newCodeAction);
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // Edit menu (placeholder)
    m_editMenu = menuBar()->addMenu(tr("&Edit"));

    // View menu (placeholder)
    m_viewMenu = menuBar()->addMenu(tr("&View"));

    // Simulation menu (placeholder)
    m_simulationMenu = menuBar()->addMenu(tr("&Simulation"));

    // Help menu (placeholder)
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createToolbars()
{
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newCodeAction);
    m_fileToolBar->addAction(m_openAction);
    m_fileToolBar->addAction(m_saveAction);
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

void MainWindow::newCodeDocument()
{
    // Get a document name
    bool ok;
    QString name = QInputDialog::getText(
        this,
        tr("New Turing Machine"),
        tr("Enter a name for the new machine:"),
        QLineEdit::Normal,
        tr("Untitled"),
        &ok
    );

    if (!ok || name.isEmpty()) {
        name = tr("Untitled");
    }

    // Create a new code document
    CodeDocument* document = DocumentManager::getInstance().createCodeDocument(name.toStdString());

    // Open it in a tab
    if (document) {
        m_tabManager->openDocument(document);
        statusBar()->showMessage(tr("Created new code document: %1").arg(name), 2000);
    }
}

void MainWindow::openDocument()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Document"),
        QString(),
        tr("Turing Machine Files (*.tm);;Tape Files (*.tape);;All Files (*)")
    );

    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    // Determine document type from extension
    if (extension == "tm") {
        CodeDocument* document = DocumentManager::getInstance().openCodeDocument(filePath.toStdString());
        if (document) {
            m_tabManager->openDocument(document);
            statusBar()->showMessage(tr("Opened code document: %1").arg(fileInfo.fileName()), 2000);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open the document"));
        }
    } else if (extension == "tape") {
        // For tape documents, we need a code document
        // In a real implementation, you would read the tape file to find the associated code document
        // For simplicity, we'll assume the code document is already open or create a new one

        CodeDocument* codeDoc = nullptr;

        // Check if we have any open code documents
        for (Document* doc : DocumentManager::getInstance().getAllDocuments()) {
            if (doc->getType() == Document::DocumentType::CODE) {
                codeDoc = static_cast<CodeDocument*>(doc);
                break;
            }
        }

        // If no code document is open, create one
        if (!codeDoc) {
            codeDoc = DocumentManager::getInstance().createCodeDocument("Untitled");
        }

        // Now open the tape document
        TapeDocument* tapeDoc = DocumentManager::getInstance().openTapeDocument(
            filePath.toStdString(),
            codeDoc
        );

        if (tapeDoc) {
            m_tabManager->openDocument(tapeDoc);
            statusBar()->showMessage(tr("Opened tape document: %1").arg(fileInfo.fileName()), 2000);
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Failed to open the tape document"));
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Unknown file type"));
    }
}

void MainWindow::saveDocument()
{
    if (!m_currentDocument) return;

    if (m_currentDocument->getFilePath().empty()) {
        saveDocumentAs();
        return;
    }

    if (DocumentManager::getInstance().saveDocument(m_currentDocument)) {
        statusBar()->showMessage(tr("Document saved"), 2000);
    } else {
        QMessageBox::warning(
            this,
            tr("Save Error"),
            tr("Failed to save the document")
        );
    }
}

void MainWindow::saveDocumentAs()
{
    if (!m_currentDocument) return;

    QString filter;
    switch (m_currentDocument->getType()) {
        case Document::DocumentType::CODE:
            filter = tr("Turing Machine Files (*.tm)");
            break;
        case Document::DocumentType::TAPE:
            filter = tr("Tape Files (*.tape)");
            break;
        default:
            filter = tr("All Files (*)");
            break;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Document As"),
        QString::fromStdString(m_currentDocument->getName()),
        filter
    );

    if (filePath.isEmpty()) return;

    // Add extension if missing
    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix().isEmpty()) {
        switch (m_currentDocument->getType()) {
            case Document::DocumentType::CODE:
                filePath += ".tm";
                break;
            case Document::DocumentType::TAPE:
                filePath += ".tape";
                break;
        }
    }

    if (DocumentManager::getInstance().saveDocumentAs(m_currentDocument, filePath.toStdString())) {
        statusBar()->showMessage(tr("Document saved as %1").arg(filePath), 2000);
    } else {
        QMessageBox::warning(
            this,
            tr("Save Error"),
            tr("Failed to save the document as %1").arg(filePath)
        );
    }
}

void MainWindow::onDocumentTabChanged(Document* document)
{
    m_currentDocument = document;
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

void MainWindow::onDocumentAdded(Document* document)
{
    // For automatic handling, open the document in a tab
    m_tabManager->openDocument(document);
}

void MainWindow::updateUIForDocument(Document* document)
{
    if (document) {
        // Update window title
        QString title = QString::fromStdString(document->getName());
        if (document->isModified()) {
            title += "*";
        }
        setWindowTitle(title + " - Turing Machine Visualizer");

        // Enable document-related actions
        m_saveAction->setEnabled(true);
        m_saveAsAction->setEnabled(true);

        // Show document type in status bar
        QString typeStr;
        switch (document->getType()) {
            case Document::DocumentType::CODE:
                typeStr = tr("Code");
                break;
            case Document::DocumentType::TAPE:
                typeStr = tr("Tape");
                break;
            default:
                typeStr = tr("Unknown");
                break;
        }

        statusBar()->showMessage(tr("%1 document: %2")
            .arg(typeStr)
            .arg(QString::fromStdString(document->getName())));
    } else {
        // No document is active
        setWindowTitle("Turing Machine Visualizer");

        // Disable document-related actions
        m_saveAction->setEnabled(false);
        m_saveAsAction->setEnabled(false);

        statusBar()->showMessage(tr("No document selected"));
    }
}