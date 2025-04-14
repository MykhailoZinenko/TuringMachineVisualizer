#pragma once

#include <string>
#include <QObject>

/**
 * Base document class for all openable documents
 */
class Document : public QObject
{
    Q_OBJECT

public:
    /**
     * Document types supported by the application
     */
    enum class DocumentType {
        CODE,       // Turing machine code (.tm)
        TAPE        // Tape visualization (.tmt)
    };

    /**
     * Constructor
     * @param type The type of document
     * @param name The display name of the document
     * @param filePath The file path, or empty if not yet saved
     */
    Document(DocumentType type, const std::string& name = "Untitled",
             const std::string& filePath = "");

    /**
     * Virtual destructor
     */
    virtual ~Document();

    /**
     * Get document type
     * @return The document type
     */
    DocumentType getType() const { return m_type; }

    /**
     * Get unique identifier for this document
     * @return The document ID
     */
    std::string getId() const { return m_id; }

    /**
     * Get document display name
     * @return The document name
     */
    std::string getName() const { return m_name; }

    /**
     * Set document display name
     * @param name The new name
     */
    void setName(const std::string& name);

    /**
     * Get file path for this document
     * @return The file path, or empty if not yet saved
     */
    std::string getFilePath() const { return m_filePath; }

    /**
     * Set file path for this document
     * @param path The new file path
     */
    void setFilePath(const std::string& path);

    /**
     * Check if document has been modified since last save
     * @return True if modified, false otherwise
     */
    bool isModified() const { return m_isModified; }

    /**
     * Set modified flag
     * @param modified The new modified state
     */
    void setModified(bool modified);

    /**
     * Save document to its current file path
     * @return True if saved successfully, false otherwise
     */
    virtual bool save();

    /**
     * Save document to a new file path
     * @param filePath The new file path
     * @return True if saved successfully, false otherwise
     */
    virtual bool saveAs(const std::string& filePath);

    /**
     * Load document from a file
     * @param filePath The file path to load from
     * @return True if loaded successfully, false otherwise
     */
    virtual bool loadFromFile(const std::string& filePath);

signals:
    /**
     * Signal emitted when the document name changes
     * @param newName The new name
     */
    void nameChanged(const std::string& newName);

    /**
     * Signal emitted when the file path changes
     * @param newPath The new file path
     */
    void filePathChanged(const std::string& newPath);

    /**
     * Signal emitted when the modified state changes
     * @param modified The new modified state
     */
    void modificationChanged(bool modified);

    /**
     * Signal emitted when the document is saved
     * @param filePath The file path it was saved to
     */
    void documentSaved(const std::string& filePath);

    /**
     * Signal emitted when document content changes
     */
    void contentChanged();

protected:
    /**
     * Generate a unique ID for this document
     * @return A unique ID string
     */
    static std::string generateUniqueId();

    /**
     * Perform actual save operation - to be implemented by subclasses
     * @param filePath The file path to save to
     * @return True if saved successfully, false otherwise
     */
    virtual bool saveToFile(const std::string& filePath) = 0;

private:
    DocumentType m_type;
    std::string m_id;      // Unique identifier
    std::string m_name;    // Display name
    std::string m_filePath; // File path
    bool m_isModified;     // Modified flag
};