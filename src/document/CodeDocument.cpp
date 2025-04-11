#include "CodeDocument.h"
#include "../project/Project.h"
#include "../model/TuringMachine.h"
#include "../parser/CodeParser.h"
#include <QDebug>

CodeDocument::CodeDocument(Project* project, const std::string& name)
    : Document(project, DocumentType::CODE, name)
{
    // Initialize with the machine's code if available
    if (project && project->getMachine()) {
        m_code = project->getMachine()->getOriginalCode();
    }
}

CodeDocument::~CodeDocument()
{
}

std::string CodeDocument::getCode() const
{
    return m_code;
}

void CodeDocument::setCode(const std::string& code)
{
    if (m_code != code) {
        m_code = code;

        // Update the machine with the new code
        if (getProject() && getProject()->getMachine()) {
            // Parse the code and update the machine
            CodeParser parser;
            bool success = parser.parseAndUpdateMachine(getProject()->getMachine(), code);

            if (!success) {
                qWarning() << "Failed to parse code";
            }

            // Store the original code in the machine
            getProject()->getMachine()->setOriginalCode(code);

            // Mark the project as modified
            getProject()->setModified(true);
        }

        emit codeChanged(m_code);
    }
}