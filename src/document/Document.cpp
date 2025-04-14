#include "Document.h"
#include <QUuid>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

Document::Document(DocumentType type, const std::string& name, const std::string& filePath)
    : m_type(type), m_id(generateUniqueId()), m_name(name), m_filePath(filePath),
      m_isModified(false)
{
    // If we have a file path, use its filename as the document name if none provided
    if (!m_filePath.empty() && m_name == "Untitled") {
        QFileInfo fileInfo(QString::fromStdString(m_filePath));
        m_name = fileInfo.baseName().toStdString();
    }
}

Document::~Document()
{
}

void Document::setName(const std::string& name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
    }
}

void Document::setFilePath(const std::string& path)
{
    if (m_filePath != path) {
        m_filePath = path;
        emit filePathChanged(m_filePath);

        // Update name if it was the default
        if (m_name == "Untitled" && !m_filePath.empty()) {
            QFileInfo fileInfo(QString::fromStdString(m_filePath));
            setName(fileInfo.baseName().toStdString());
        }
    }
}

void Document::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modificationChanged(m_isModified);
    }
}

bool Document::save()
{
    if (m_filePath.empty()) {
        qWarning() << "Cannot save document with empty file path";
        return false;
    }

    if (saveToFile(m_filePath)) {
        setModified(false);
        emit documentSaved(m_filePath);
        return true;
    }

    return false;
}

bool Document::saveAs(const std::string& filePath)
{
    if (saveToFile(filePath)) {
        setFilePath(filePath);
        setModified(false);
        emit documentSaved(filePath);
        return true;
    }

    return false;
}

bool Document::loadFromFile(const std::string& filePath)
{
    // Default implementation - subclasses should override
    qWarning() << "Document::loadFromFile not implemented by subclass";
    return false;
}

std::string Document::generateUniqueId()
{
    // Generate a unique ID using QUuid
    QString uuid = QUuid::createUuid().toString();

    // Remove curly braces from the UUID
    uuid.remove('{').remove('}');

    return "doc_" + uuid.toStdString();
}