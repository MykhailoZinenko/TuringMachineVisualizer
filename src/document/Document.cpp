#include "Document.h"
#include "../project/Project.h"
#include <QUuid>

Document::Document(Project* project, DocumentType type, const std::string& name)
    : m_project(project), m_type(type), m_id(generateUniqueId()), m_name(name)
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

        // Mark the project as modified
        if (m_project) {
            m_project->setModified(true);
        }
    }
}

std::string Document::generateUniqueId()
{
    // Generate a unique ID using QUuid
    QString uuid = QUuid::createUuid().toString();

    // Remove curly braces from the UUID
    uuid.remove('{').remove('}');

    return "doc_" + uuid.toStdString();
}