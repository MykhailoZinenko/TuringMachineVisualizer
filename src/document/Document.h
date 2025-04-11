#pragma once

#include <string>
#include <QObject>

// Forward declaration
class Project;

/**
 * Base document class for all openable documents
 */
class Document : public QObject
{
    Q_OBJECT

public:
    enum class DocumentType {
        CODE,       // Turing machine code
        TAPE        // Tape visualization
    };

    Document(Project* project, DocumentType type, const std::string& name = "Untitled");
    virtual ~Document();

    // Document properties
    DocumentType getType() const { return m_type; }

    std::string getId() const { return m_id; }

    std::string getName() const { return m_name; }
    void setName(const std::string& name);

    // Project access
    Project* getProject() const { return m_project; }

    signals:
        void nameChanged(const std::string& newName);

protected:
    static std::string generateUniqueId();

private:
    Project* m_project;    // Non-owning reference to parent project
    DocumentType m_type;
    std::string m_id;      // Unique identifier
    std::string m_name;
};