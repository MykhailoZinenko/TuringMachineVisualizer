#include "TapeDocument.h"
#include "CodeDocument.h"
#include "../model/Tape.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDebug>

#include "model/TuringMachine.h"

TapeDocument::TapeDocument(CodeDocument* codeDocument, const std::string& name)
    : Document(DocumentType::TAPE, name),
      m_codeDocument(codeDocument),
      m_initialHeadPosition(0)
{
    // Create a new tape
    m_tape = std::make_unique<Tape>();

    if (codeDocument && codeDocument->getMachine()) {
        codeDocument->getMachine()->setActiveTape(m_tape.get());
    }
}

TapeDocument::~TapeDocument()
{
}

void TapeDocument::setInitialContent(const std::string& content)
{
    m_initialContent = content;
    m_tape->setInitialContent(content);
    setModified(true);
}

void TapeDocument::setInitialHeadPosition(int position)
{
    m_initialHeadPosition = position;
    m_tape->setHeadPosition(position);
    setModified(true);
}

bool TapeDocument::saveToFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << QString::fromStdString(filePath);
        return false;
    }
    
    // Create a JSON object for the tape configuration
    QJsonObject json;
    json["name"] = QString::fromStdString(getName());
    json["initialContent"] = QString::fromStdString(m_initialContent);
    json["initialHeadPosition"] = m_initialHeadPosition;
    
    // If code document has a file path, store reference
    if (!m_codeDocument->getFilePath().empty()) {
        json["codeFile"] = QString::fromStdString(m_codeDocument->getFilePath());
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
    
    // Update file path and reset modified flag
    setFilePath(filePath);
    setModified(false);
    
    // Update name from file name
    QFileInfo fileInfo(file);
    setName(fileInfo.baseName().toStdString());
    
    return true;
}

bool TapeDocument::loadFromFile(const std::string& filePath)
{
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << QString::fromStdString(filePath);
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in tape file:" << QString::fromStdString(filePath);
        return false;
    }
    
    QJsonObject json = doc.object();
    
    // Load tape configuration
    if (json.contains("name")) {
        setName(json["name"].toString().toStdString());
    }
    
    if (json.contains("initialContent")) {
        m_initialContent = json["initialContent"].toString().toStdString();
        m_tape->setInitialContent(m_initialContent);
    }
    
    if (json.contains("initialHeadPosition")) {
        m_initialHeadPosition = json["initialHeadPosition"].toInt();
        m_tape->setHeadPosition(m_initialHeadPosition);
    }
    
    // Update file path and reset modified flag
    setFilePath(filePath);
    setModified(false);
    
    return true;
}