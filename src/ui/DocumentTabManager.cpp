#include "DocumentTabManager.h"
#include "../document/Document.h"
#include "../document/CodeDocument.h"
#include "../document/TapeDocument.h"
#include "../project/Project.h"
#include "document/DocumentView.h"
#include "document/CodeEditorView.h"
#include "document/TapeVisualizationView.h"
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTabBar>

DocumentTabManager::DocumentTabManager(QWidget* parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);

    connect(this, &QTabWidget::tabCloseRequested, this, &DocumentTabManager::onTabCloseRequested);
    connect(this, &QTabWidget::currentChanged, this, &DocumentTabManager::onCurrentChanged);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &DocumentTabManager::onTabContextMenu);

    setupContextMenu();
}

DocumentTabManager::~DocumentTabManager()
{
}

void DocumentTabManager::setupContextMenu()
{
    m_tabContextMenu = new QMenu(this);

    QAction* closeTabAction = m_tabContextMenu->addAction(tr("Close"));
    connect(closeTabAction, &QAction::triggered, this, &DocumentTabManager::closeCurrentTab);

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
}

void DocumentTabManager::openDocument(Document* document)
{
    if (!document) return;

    // Check if already open
    int existingIndex = findTabIndex(document);
    if (existingIndex >= 0) {
        setCurrentIndex(existingIndex);
        return;
    }

    // Create view based on document type
    DocumentView* view = createViewForDocument(document);
    if (!view) return;

    // Get the appropriate name for the tab
    QString tabName = QString::fromStdString(document->getName());

    // Add tab
    int index = addTab(view, tabName);
    setCurrentIndex(index);

    // Store mapping
    m_documentViews[document] = view;

    // Connect signals
    connect(view, &DocumentView::viewModified, this, &DocumentTabManager::onDocumentModified);
    connect(document, &Document::nameChanged, [this, view](const std::string&) {
        updateTabText(view);
    });

    // If document is part of a project, connect to project signals
    if (document->getProject()) {
        connect(document->getProject(), &Project::tapeAdded,
                this, &DocumentTabManager::onTapeAdded);
    }
}

void DocumentTabManager::openProject(Project* project)
{
    if (!project) return;

    // Open code document
    if (project->getCodeDocument()) {
        openDocument(project->getCodeDocument());
    }

    // Open all tape documents
    for (TapeDocument* tapeDoc : project->getAllTapes()) {
        openDocument(tapeDoc);
    }
}

void DocumentTabManager::closeDocument(Document* document)
{
    int index = findTabIndex(document);
    if (index >= 0) {
        onTabCloseRequested(index);
    }
}

void DocumentTabManager::closeCurrentTab()
{
    int index = currentIndex();
    if (index >= 0) {
        onTabCloseRequested(index);
    }
}

int DocumentTabManager::findTabIndex(Document* document) const
{
    auto it = m_documentViews.find(document);
    if (it != m_documentViews.end()) {
        return indexOf(it->second);
    }
    return -1;
}

DocumentView* DocumentTabManager::createViewForDocument(Document* document)
{
    switch (document->getType()) {
        case Document::DocumentType::CODE: {
            CodeDocument* codeDoc = static_cast<CodeDocument*>(document);
            return new CodeEditorView(codeDoc, this);
        }
        case Document::DocumentType::TAPE: {
            TapeDocument* tapeDoc = static_cast<TapeDocument*>(document);
            return new TapeVisualizationView(tapeDoc, this);
        }
        default:
            return nullptr;
    }
}

void DocumentTabManager::onTabCloseRequested(int index)
{
    DocumentView* view = qobject_cast<DocumentView*>(widget(index));
    if (!view) return;

    Document* document = view->getDocument();
    if (!document) return;

    // Check if document is modified
    if (document->getProject() && document->getProject()->isModified()) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this,
            tr("Unsaved Changes"),
            tr("Project '%1' has unsaved changes. Save before closing?")
                .arg(QString::fromStdString(document->getProject()->getName())),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        if (result == QMessageBox::Cancel) {
            return;
        }

        if (result == QMessageBox::Save) {
            // Save project logic would go here
            // For now, we'll just mark it as not modified
            document->getProject()->setModified(false);
        }
    }

    // Remove from mapping
    m_documentViews.erase(document);

    // Remove the tab
    removeTab(index);

    // Delete the view
    delete view;

    // Emit signal
    emit documentTabClosed(document);
}

void DocumentTabManager::onCurrentChanged(int index)
{
    if (index < 0) {
        emit documentTabChanged(nullptr);
        return;
    }

    DocumentView* view = qobject_cast<DocumentView*>(widget(index));
    if (view && view->getDocument()) {
        emit documentTabChanged(view->getDocument());
    } else {
        emit documentTabChanged(nullptr);
    }
}

void DocumentTabManager::onTabContextMenu(const QPoint& point)
{
    int tabIndex = tabBar()->tabAt(point);
    if (tabIndex >= 0) {
        m_tabContextMenu->exec(mapToGlobal(point));
    }
}

void DocumentTabManager::onDocumentModified()
{
    DocumentView* view = qobject_cast<DocumentView*>(sender());
    if (view) {
        updateTabText(view);
    }
}

void DocumentTabManager::onTapeAdded(TapeDocument* tapeDoc)
{
    if (tapeDoc) {
        openDocument(tapeDoc);
    }
}

void DocumentTabManager::updateTabText(DocumentView* view)
{
    int index = indexOf(view);
    if (index < 0) return;

    Document* document = view->getDocument();
    if (!document) return;

    QString tabName = QString::fromStdString(document->getName());
    if (document->getProject() && document->getProject()->isModified()) {
        tabName += "*";
    }
    
    setTabText(index, tabName);
}