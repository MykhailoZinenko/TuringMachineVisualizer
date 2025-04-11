#include "CodeDocument.h"
#include "../model/TuringMachine.h"
#include "../parser/CodeParser.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

CodeDocument::CodeDocument(const std::string& name)
    : Document(DocumentType::CODE, name)
{
    m_machine = std::make_unique<TuringMachine>(name);
}

CodeDocument::~CodeDocument()
{
}

std::string CodeDocument::getCode() const
{
    // Return the stored code
    if (!m_code.empty()) {
        return m_code;
    }
    
    // Fallback to machine's original code
    return m_machine->getOriginalCode();
}

void CodeDocument::setCode(const std::string& code)
{
    if (m_code != code) {
        m_code = code;

        // Use the parser to update the machine from the code
        CodeParser parser;
        bool success = parser.parseAndUpdateMachine(m_machine.get(), m_code);

        if (!success) {
            qWarning() << "Failed to parse code and update machine";
        }

        // Store the code in the machine for serialization
        m_machine->setOriginalCode(m_code);

        setModified(true);
        emit codeChanged(m_code);
    }
}
bool CodeDocument::saveToFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << QString::fromStdString(filePath);
        return false;
    }
    
    // Create a JSON representation of the document
    std::string jsonStr = m_machine->toJson();
    
    QTextStream out(&file);
    out << QString::fromStdString(jsonStr);
    file.close();
    
    // Update file path and reset modified flag
    setFilePath(filePath);
    setModified(false);
    
    // Update name from file name
    QFileInfo fileInfo(file);
    setName(fileInfo.baseName().toStdString());
    
    return true;
}

bool CodeDocument::loadFromFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << QString::fromStdString(filePath);
        return false;
    }
    
    QTextStream in(&file);
    std::string jsonStr = in.readAll().toStdString();
    file.close();
    
    try {
        // Create machine from JSON
        auto loadedMachine = TuringMachine::fromJson(jsonStr);
        if (!loadedMachine) {
            throw std::runtime_error("Failed to create machine from file");
        }
        
        // Update our machine
        m_machine = std::move(loadedMachine);
        
        // Get the original code
        m_code = m_machine->getOriginalCode();
        
        // Update file path and reset modified flag
        setFilePath(filePath);
        setModified(false);
        
        // Update name from file name
        QFileInfo fileInfo(file);
        setName(fileInfo.baseName().toStdString());
        
        emit codeChanged(m_code);
        
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Error loading machine:" << e.what();
        return false;
    }
}