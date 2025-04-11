#pragma once

#include "Document.h"
#include <memory>
#include <string>

class Tape;
class CodeDocument;

// Document representing a tape visualization linked to a code document
class TapeDocument : public Document
{
    Q_OBJECT

public:
    TapeDocument(CodeDocument* codeDocument, const std::string& name = "Tape");
    ~TapeDocument() override;

    // Tape access
    Tape* getTape() const { return m_tape.get(); }
    
    // Code document access
    CodeDocument* getCodeDocument() const { return m_codeDocument; }
    
    // Tape configuration
    void setInitialContent(const std::string& content);
    void setInitialHeadPosition(int position);
    
    // Serialization
    bool saveToFile(const std::string& filePath) override;
    bool loadFromFile(const std::string& filePath) override;

private:
    std::unique_ptr<Tape> m_tape;
    CodeDocument* m_codeDocument;  // Non-owning reference
    std::string m_initialContent;
    int m_initialHeadPosition;
};