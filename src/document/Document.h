#pragma once

#include <string>
#include <memory>
#include <QObject>

// Base document class for all openable documents
class Document : public QObject
{
    Q_OBJECT

public:
    enum class DocumentType {
        CODE,       // Turing machine code
        TAPE        // Tape visualization
    };

    Document(DocumentType type, const std::string& name = "Untitled");
    virtual ~Document();

    // Document properties
    DocumentType getType() const { return m_type; }
    std::string getName() const { return m_name; }
    void setName(const std::string& name);
    
    std::string getFilePath() const { return m_filePath; }
    void setFilePath(const std::string& path);
    
    bool isModified() const { return m_isModified; }
    void setModified(bool modified);

    // Serialization
    virtual bool saveToFile(const std::string& filePath) = 0;
    virtual bool loadFromFile(const std::string& filePath) = 0;

signals:
    void nameChanged(const std::string& newName);
    void modificationChanged(bool modified);

private:
    DocumentType m_type;
    std::string m_name;
    std::string m_filePath;
    bool m_isModified;
};