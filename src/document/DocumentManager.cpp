#include "DocumentManager.h"
#include "Document.h"
#include "CodeDocument.h"
#include "TapeDocument.h"
#include <QFileInfo>
#include <QDebug>

DocumentManager& DocumentManager::getInstance()
{
    static DocumentManager instance;
    return instance;
}

DocumentManager::DocumentManager()
{
}

DocumentManager::~DocumentManager()
{
}

CodeDocument* DocumentManager::createCodeDocument(const std::string& name)
{
    auto document = std::make_unique<CodeDocument>(name);
    CodeDocument* documentPtr = document.get();
    
    m_documents.push_back(std::move(document));
    emit documentAdded(documentPtr);
    
    return documentPtr;
}

TapeDocument* DocumentManager::createTapeDocument(CodeDocument* codeDocument, const std::string& name)
{
    if (!codeDocument) {
        qWarning() << "Cannot create tape document without a code document";
        return nullptr;
    }
    
    auto document = std::make_unique<TapeDocument>(codeDocument, name);
    TapeDocument* documentPtr = document.get();
    
    m_documents.push_back(std::move(document));
    emit documentAdded(documentPtr);
    
    return documentPtr;
}

bool DocumentManager::saveDocument(Document* document)
{
    if (!document) return false;
    
    // If document has no path, prompt for one (this would be handled by UI)
    if (document->getFilePath().empty()) {
        return false; // Can't save without a path
    }
    
    return document->saveToFile(document->getFilePath());
}

bool DocumentManager::saveDocumentAs(Document* document, const std::string& filePath)
{
    if (!document) return false;
    
    if (document->saveToFile(filePath)) {
        emit documentSaved(document);
        return true;
    }
    return false;
}

bool DocumentManager::closeDocument(Document* document)
{
    if (!document) return false;
    
    auto it = std::find_if(m_documents.begin(), m_documents.end(),
                          [document](const auto& doc) { return doc.get() == document; });
    
    if (it != m_documents.end()) {
        // Signal before removing
        emit documentClosed(document);
        m_documents.erase(it);
        return true;
    }
    return false;
}

CodeDocument* DocumentManager::openCodeDocument(const std::string& filePath)
{
    // Check if already open
    Document* existingDoc = findDocumentByPath(filePath);
    if (existingDoc && existingDoc->getType() == Document::DocumentType::CODE) {
        return static_cast<CodeDocument*>(existingDoc);
    }
    
    // Create new document
    auto document = std::make_unique<CodeDocument>();
    CodeDocument* documentPtr = document.get();
    
    if (documentPtr->loadFromFile(filePath)) {
        m_documents.push_back(std::move(document));
        emit documentAdded(documentPtr);
        return documentPtr;
    }
    
    return nullptr;
}

TapeDocument* DocumentManager::openTapeDocument(const std::string& filePath, CodeDocument* codeDocument)
{
    // Check if already open
    Document* existingDoc = findDocumentByPath(filePath);
    if (existingDoc && existingDoc->getType() == Document::DocumentType::TAPE) {
        return static_cast<TapeDocument*>(existingDoc);
    }
    
    // Need a code document
    if (!codeDocument) {
        qWarning() << "Cannot open tape document without a code document";
        return nullptr;
    }
    
    // Create new document
    auto document = std::make_unique<TapeDocument>(codeDocument);
    TapeDocument* documentPtr = document.get();
    
    if (documentPtr->loadFromFile(filePath)) {
        m_documents.push_back(std::move(document));
        emit documentAdded(documentPtr);
        return documentPtr;
    }
    
    return nullptr;
}

std::vector<Document*> DocumentManager::getAllDocuments() const
{
    std::vector<Document*> result;
    for (const auto& doc : m_documents) {
        result.push_back(doc.get());
    }
    return result;
}

Document* DocumentManager::findDocumentByPath(const std::string& filePath) const
{
    for (const auto& doc : m_documents) {
        if (doc->getFilePath() == filePath) {
            return doc.get();
        }
    }
    return nullptr;
}