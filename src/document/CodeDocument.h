#pragma once

#include "Document.h"
#include <string>

/**
 * Document representing the code for a Turing machine
 */
class CodeDocument : public Document
{
    Q_OBJECT

public:
    CodeDocument(Project* project, const std::string& name = "Code");
    ~CodeDocument() override;

    // Code handling
    std::string getCode() const;
    void setCode(const std::string& code);

    signals:
        void codeChanged(const std::string& newCode);

private:
    std::string m_code;
};