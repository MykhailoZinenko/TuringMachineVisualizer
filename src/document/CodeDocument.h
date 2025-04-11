#pragma once

#include "Document.h"
#include <memory>

class TuringMachine;

// Document representing a Turing machine code file
class CodeDocument : public Document
{
    Q_OBJECT

public:
    CodeDocument(const std::string& name = "Untitled");
    ~CodeDocument() override;

    // Machine access
    TuringMachine* getMachine() const { return m_machine.get(); }

    // Code handling
    std::string getCode() const;
    void setCode(const std::string& code);

    // Serialization
    bool saveToFile(const std::string& filePath) override;
    bool loadFromFile(const std::string& filePath) override;

    signals:
        void codeChanged(const std::string& newCode);

private:
    std::unique_ptr<TuringMachine> m_machine;
    std::string m_code;
};