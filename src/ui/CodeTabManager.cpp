#include "CodeTabManager.h"
#include "CodeEditorView.h"
#include "../document/CodeDocument.h"
#include "../core/SessionManager.h"

#include <QMenu>
#include <QAction>
#include <QTabBar>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

CodeTabManager::CodeTabManager(QWidget* parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);
    
    connect(this, &QTabWidget::tabCloseRequested, this, &CodeTabManager::onTabCloseRequested);
    connect(this, &QTabWidget::currentChanged, this, &CodeTabManager::onCurrentChanged);
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &CodeTabManager::onTabContextMenu);
    
    setupContextMenu();
}

CodeTabManager::~CodeTabManager()
{
}

void CodeTabManager::setupContextMenu()
{
    m_tabContextMenu = new QMenu(this);
    
    QAction* closeTabAction = m_tabContextMenu->addAction(tr("Close"));
    connect(closeTabAction, &QAction::triggered, this, &CodeTabManager::closeCurrentTab);
    
    QAction* closeOtherTabsAction = m_tabContextMenu->addAction(tr("Close Others"));
    connect(closeOtherTabsAction, &QAction::triggered, [this]() {
        int currentIdx = currentIndex();
        if (currentIdx < 0) return;
        
        for (int i = count() - 1; i >= 0; --i) {
            if (i != currentIdx) {
                onTabCloseRequested(i);
            }
        }
    });
    
    QAction* closeAllTabsAction = m_tabContextMenu->addAction(tr("Close All"));
    connect(closeAllTabsAction, &QAction::triggered, [this]() {
        while (count() > 0) {
            onTabCloseRequested(0);
        }
    });
    
    m_tabContextMenu->addSeparator();
    
    QAction* saveAction = m_tabContextMenu->addAction(tr("Save"));
    connect(saveAction, &QAction::triggered, this, &CodeTabManager::saveCurrentDocument);
    
    QAction* saveAsAction = m_tabContextMenu->addAction(tr("Save As..."));
    connect(saveAsAction, &QAction::triggered, this, &CodeTabManager::saveCurrentDocumentAs);
}

bool CodeTabManager::openDocument(CodeDocument* document)
{
    if (!document) {
        qWarning() << "Attempt to open null document";
        return false;
    }
    
    // Check if already open
    int existingIndex = findTabIndex(document);
    if (existingIndex >= 0) {
        setCurrentIndex(existingIndex);
        return true;
    }
    
    // Create view for document
    CodeEditorView* view = createViewForDocument(document);
    if (!view) {
        qWarning() << "Failed to create view for document";
        return false;
    }
    
    // Get display name for tab
    QString displayName = QFileInfo(QString::fromStdString(document->getFilePath())).fileName();
    if (displayName.isEmpty()) {
        displayName = QString::fromStdString(document->getName());
    }
    
    // Add tab
    int index = addTab(view, displayName);
    setCurrentIndex(index);
    
    // Store mapping
    m_documentViews[document] = view;
    
    // Connect document signals
    connect(document, &Document::nameChanged, [this, view](const std::string&) {
        updateTabText(view);
    });
    
    connect(document, &Document::filePathChanged, [this, view](const std::string&) {
        updateTabText(view);
    });
    
    connect(document, &Document::modificationChanged, [this, view](bool) {
        updateTabText(view);
    });
    
    // Connect view signals
    connect(view, &CodeEditorView::viewModified, this, &CodeTabManager::onDocumentModified);
    
    // Emit signal
    emit documentOpened(document);
    
    // Set as active document in SessionManager
    SessionManager::getInstance().setActiveCodeDocument(document);
    
    return true;
}

bool CodeTabManager::isDocumentOpen(CodeDocument* document) const
{
    return m_documentViews.find(document) != m_documentViews.end();
}

CodeDocument* CodeTabManager::findDocumentByPath(const std::string& filePath) const
{
    for (const auto& pair : m_documentViews) {
        if (pair.first->getFilePath() == filePath) {
            return pair.first;
        }
    }
    return nullptr;
}

CodeDocument* CodeTabManager::getActiveDocument() const
{
    int index = currentIndex();
    if (index < 0) {
        return nullptr;
    }
    
    QWidget* widget = this->widget(index);
    CodeEditorView* view = qobject_cast<CodeEditorView*>(widget);
    if (!view) {
        return nullptr;
    }
    
    for (const auto& pair : m_documentViews) {
        if (pair.second == view) {
            return pair.first;
        }
    }
    
    return nullptr;
}

CodeDocument* CodeTabManager::createDocument(const std::string& filePath)
{
    // Check if already open
    if (!filePath.empty()) {
        CodeDocument* existingDoc = findDocumentByPath(filePath);
        if (existingDoc) {
            openDocument(existingDoc);
            return existingDoc;
        }
    }
    
    // Get document name from file path, if provided
    std::string docName = "Untitled";
    if (!filePath.empty()) {
        QFileInfo fileInfo(QString::fromStdString(filePath));
        docName = fileInfo.baseName().toStdString();
    }
    
    // Create a new document
    auto document = new CodeDocument(docName, filePath);
    
    // If file path provided, load it
    if (!filePath.empty() && !document->loadFromFile(filePath)) {
        delete document;
        return nullptr;
    }
    
    // Open the document in a tab
    if (!openDocument(document)) {
        delete document;
        return nullptr;
    }
    
    return document;
}

bool CodeTabManager::closeDocument(CodeDocument* document)
{
    if (!document) {
        return false;
    }
    
    // Find the view
    auto it = m_documentViews.find(document);
    if (it == m_documentViews.end()) {
        return false;
    }
    
    // Check for unsaved changes
    if (!checkUnsavedChanges(document)) {
        return false;
    }
    
    // Get the view and tab index
    CodeEditorView* view = it->second;
    int index = indexOf(view);
    
    // Remove the mapping
    m_documentViews.erase(it);
    
    // Remove the tab
    removeTab(index);
    
    // Delete the view
    delete view;
    
    // Emit signal
    emit documentClosed(document);
    
    // Update SessionManager active document if this was the active one
    if (SessionManager::getInstance().getActiveCodeDocument() == document) {
        SessionManager::getInstance().setActiveCodeDocument(getActiveDocument());
    }
    
    return true;
}

bool CodeTabManager::closeCurrentTab()
{
    CodeDocument* document = getActiveDocument();
    if (document) {
        return closeDocument(document);
    }
    return false;
}

bool CodeTabManager::saveCurrentDocument()
{
    CodeDocument* document = getActiveDocument();
    if (!document) {
        return false;
    }
    
    // If no file path, prompt for one
    if (document->getFilePath().empty()) {
        return saveCurrentDocumentAs();
    }
    
    // Save the document
    if (document->save()) {
        updateTabText(m_documentViews[document]);
        emit documentSaved(document);
        return true;
    }
    
    return false;
}

bool CodeTabManager::saveCurrentDocumentAs()
{
    CodeDocument* document = getActiveDocument();
    if (!document) {
        return false;
    }
    
    // Show save dialog
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Code File"),
                                                 QString::fromStdString(document->getName()),
                                                 tr("Turing Machine Files (*.tm)"));
    if (filePath.isEmpty()) {
        return false;
    }
    
    // Ensure it has .tm extension
    if (!filePath.toLower().endsWith(".tm")) {
        filePath += ".tm";
    }
    
    // Save the document
    if (document->saveAs(filePath.toStdString())) {
        updateTabText(m_documentViews[document]);
        emit documentSaved(document);
        return true;
    }
    
    return false;
}

void CodeTabManager::onTabCloseRequested(int index)
{
    QWidget* widget = this->widget(index);
    CodeEditorView* view = qobject_cast<CodeEditorView*>(widget);
    if (!view) {
        return;
    }
    
    // Find the document
    CodeDocument* document = nullptr;
    for (const auto& pair : m_documentViews) {
        if (pair.second == view) {
            document = pair.first;
            break;
        }
    }
    
    if (document) {
        closeDocument(document);
    }
}

void CodeTabManager::onCurrentChanged(int index)
{
    if (index < 0) {
        emit activeDocumentChanged(nullptr);
        SessionManager::getInstance().setActiveCodeDocument(nullptr);
        return;
    }
    
    CodeDocument* document = getActiveDocument();
    if (document) {
        emit activeDocumentChanged(document);
        SessionManager::getInstance().setActiveCodeDocument(document);
    }
}

void CodeTabManager::onTabContextMenu(const QPoint& point)
{
    int tabIndex = tabBar()->tabAt(point);
    if (tabIndex >= 0) {
        m_tabContextMenu->popup(mapToGlobal(point));
    }
}

void CodeTabManager::onDocumentModified()
{
    CodeEditorView* view = qobject_cast<CodeEditorView*>(sender());
    if (view) {
        updateTabText(view);
    }
}

void CodeTabManager::updateTabText(CodeEditorView* view)
{
    int index = indexOf(view);
    if (index < 0) {
        return;
    }
    
    // Find the document
    CodeDocument* document = nullptr;
    for (const auto& pair : m_documentViews) {
        if (pair.second == view) {
            document = pair.first;
            break;
        }
    }
    
    if (!document) {
        return;
    }
    
    // Update tab text
    QString displayName = QFileInfo(QString::fromStdString(document->getFilePath())).fileName();
    if (displayName.isEmpty()) {
        displayName = QString::fromStdString(document->getName());
    }
    
    if (document->isModified()) {
        displayName += " *";
    }
    
    setTabText(index, displayName);
}

int CodeTabManager::findTabIndex(CodeDocument* document) const
{
    auto it = m_documentViews.find(document);
    if (it != m_documentViews.end()) {
        return indexOf(it->second);
    }
    return -1;
}

bool CodeTabManager::checkUnsavedChanges(CodeDocument* document)
{
    if (document && document->isModified()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("The document '%1' has unsaved changes. Do you want to save it before closing?")
                .arg(QString::fromStdString(document->getName())),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (result == QMessageBox::Cancel) {
            return false;
        }
        
        if (result == QMessageBox::Save) {
            if (document->getFilePath().empty()) {
                // Show save dialog
                QString filePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Code File"),
                    QString::fromStdString(document->getName()),
                    tr("Turing Machine Files (*.tm)")
                );
                
                if (filePath.isEmpty()) {
                    return false;
                }
                
                // Ensure it has .tm extension
                if (!filePath.toLower().endsWith(".tm")) {
                    filePath += ".tm";
                }
                
                return document->saveAs(filePath.toStdString());
            } else {
                return document->save();
            }
        }
    }
    
    return true;
}

CodeEditorView* CodeTabManager::createViewForDocument(CodeDocument* document)
{
    if (!document) {
        return nullptr;
    }
    
    return new CodeEditorView(document);
}