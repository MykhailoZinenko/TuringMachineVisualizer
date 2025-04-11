#pragma once

#include <QWidget>
#include <string>

class Document;

// Base class for all document views (tabs)
class DocumentView : public QWidget
{
    Q_OBJECT

public:
    DocumentView(Document* document, QWidget* parent = nullptr);
    virtual ~DocumentView();

    Document* getDocument() const { return m_document; }

    // Update the view from the document
    virtual void updateFromDocument() = 0;

    signals:
        void viewModified();
    void viewSaved();
    void viewClosed();

protected:
    Document* m_document;
};