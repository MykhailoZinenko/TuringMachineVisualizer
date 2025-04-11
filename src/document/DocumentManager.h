#pragma once

#include <vector>
#include <memory>
#include <string>
#include <QObject>

class Document;
class CodeDocument;
class TapeDocument;

// Singleton class to manage all open documents
class DocumentManager : public QObject
{
    Q_OBJECT

public:
    static DocumentManager& getInstance();

    // Document creation
    CodeDocument* createCodeDocument(const std::string& name = "Untitled");
    TapeDocument* createTapeDocument(CodeDocument* codeDocument, const std::string& name = "Tape");

    // Document operations
    bool saveDocument(Document* document);
    bool saveDocumentAs(Document* document, const std::string& filePath);
    bool closeDocument(Document* document);

    // Document loading
    CodeDocument* openCodeDocument(const std::string& filePath);
    TapeDocument* openTapeDocument(const std::string& filePath, CodeDocument* codeDocument = nullptr);

    // Document access
    std::vector<Document*> getAllDocuments() const;
    Document* findDocumentByPath(const std::string& filePath) const;

    signals:
        void documentAdded(Document* document);
    void documentClosed(Document* document);
    void documentSaved(Document* document);

private:
    DocumentManager();
    ~DocumentManager();

    // Prevent copying
    DocumentManager(const DocumentManager&) = delete;
    DocumentManager& operator=(const DocumentManager&) = delete;

    std::vector<std::unique_ptr<Document>> m_documents;
};