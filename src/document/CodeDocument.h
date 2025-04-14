#pragma once

#include "Document.h"
#include <memory>
#include <string>
#include <QObject>

// Forward declarations
class TuringMachine;
class QFileSystemWatcher;

/**
 * Document representing the code for a Turing machine (.tm file)
 */
class CodeDocument : public Document
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param name The display name of the document
     * @param filePath The file path, or empty if not yet saved
     */
    CodeDocument(const std::string& name = "Untitled", const std::string& filePath = "");

    /**
     * Destructor
     */
    ~CodeDocument() override;

    /**
     * Get the code content
     * @return The code as a string
     */
    std::string getCode() const;

    /**
    * Set the code content
    * @param code The new code
    * @param shouldUpdateMachine Whether to update the machine immediately
    */
    void setCode(const std::string& code, bool shouldUpdateMachine = true);

    /**
     * Get the Turing machine associated with this code
     * @return Pointer to the machine
     */
    TuringMachine* getMachine() const;

    /**
     * Load document from a file
     * @param filePath The file path to load from
     * @return True if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filePath) override;

    /**
     * Update the Turing machine from the current code
     * @return True if updated successfully, false if there were errors
     */
    bool updateMachine();

signals:
    /**
     * Signal emitted when the Turing machine has been updated
     * @param machine The updated machine
     */
    void machineUpdated(TuringMachine* machine);

private slots:
    /**
     * Handle file changed notifications from the FileWatcher
     * @param filePath The path to the changed file
     */
    void onFileChanged(const QString& filePath);

protected:
    /**
     * Save to a file
     * @param filePath The file path to save to
     * @return True if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filePath) override;

private:
    std::string m_code;
    std::unique_ptr<TuringMachine> m_machine;

    /**
     * Set up file watching for this document
     */
    void setupFileWatching();

    /**
     * Remove file watching for this document
     */
    void removeFileWatching();
};