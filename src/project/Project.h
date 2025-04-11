#pragma once

#include <memory>
#include <string>
#include <vector>
#include <QObject>

// Forward declarations
class TuringMachine;
class CodeDocument;
class TapeDocument;

/**
 * Project class that contains a TuringMachine and associated documents
 */
class Project : public QObject
{
    Q_OBJECT

public:
    Project(const std::string& name = "Untitled");
    ~Project();

    // Project properties
    std::string getName() const;
    void setName(const std::string& name);

    std::string getFilePath() const;
    void setFilePath(const std::string& path);

    bool isModified() const;
    void setModified(bool modified);

    // Machine access
    TuringMachine* getMachine() { return m_machine.get(); }

    // Document management
    CodeDocument* getCodeDocument() { return m_codeDocument.get(); }

    TapeDocument* createTape(const std::string& name);
    TapeDocument* getTape(const std::string& id) const;
    std::vector<TapeDocument*> getAllTapes() const;

    // File operations
    bool saveToFile(const std::string& path);
    static std::unique_ptr<Project> loadFromFile(const std::string& path);

    signals:
        void nameChanged(const std::string& newName);
    void modificationChanged(bool modified);
    void tapeAdded(TapeDocument* tape);
    void tapeRemoved(TapeDocument* tape);

private:
    std::string m_name;
    std::string m_filePath;
    bool m_isModified;

    std::unique_ptr<TuringMachine> m_machine;
    std::unique_ptr<CodeDocument> m_codeDocument;
    std::vector<std::unique_ptr<TapeDocument>> m_tapeDocuments;

    std::string generateUniqueTapeId() const;
};