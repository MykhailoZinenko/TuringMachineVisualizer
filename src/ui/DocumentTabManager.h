#pragma once

#include <QTabWidget>
#include <memory>
#include <map>

class Document;
class DocumentView;
class QMenu;

// Manages document tabs in the main window
class DocumentTabManager : public QTabWidget
{
    Q_OBJECT

public:
    explicit DocumentTabManager(QWidget* parent = nullptr);
    ~DocumentTabManager();

    // Open document in a tab
    void openDocument(Document* document);

    // Close specific document
    void closeDocument(Document* document);

    // Close current tab
    void closeCurrentTab();

    // Find tab for document
    int findTabIndex(Document* document) const;

    signals:
        void documentTabClosed(Document* document);
    void documentTabChanged(Document* document);

    private slots:
        void onTabCloseRequested(int index);
    void onCurrentChanged(int index);
    void onTabContextMenu(const QPoint& point);
    void onDocumentModified();

private:
    std::map<Document*, DocumentView*> m_documentViews;
    QMenu* m_tabContextMenu;

    void setupContextMenu();
    void updateTabText(DocumentView* view);
    DocumentView* createViewForDocument(Document* document);
};