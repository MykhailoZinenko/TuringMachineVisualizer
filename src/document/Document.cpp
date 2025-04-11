#include "Document.h"

Document::Document(DocumentType type, const std::string& name)
    : m_type(type), m_name(name), m_isModified(false)
{
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
    m_filePath = path;
}

void Document::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modificationChanged(m_isModified);
    }
}