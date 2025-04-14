#pragma once

#include <QWidget>
#include <QStringList>
#include <QMap>
#include <memory>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QGroupBox;
class QPushButton;
class Document;
class CodeDocument;
class TapeDocument;

/**
 * Widget that manages lists of code and tape files
 */
class FileListManager : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent Parent widget
     */
    explicit FileListManager(QWidget* parent = nullptr);
    
    /**
     * Destructor
     */
    ~FileListManager();
    
    /**
     * Add a code file to the list
     * @param path The file path to add
     * @return True if added successfully, false otherwise
     */
    bool addCodeFile(const QString& path);
    
    /**
     * Add a tape file to the list
     * @param path The file path to add
     * @return True if added successfully, false otherwise
     */
    bool addTapeFile(const QString& path);
    
    /**
     * Get the list of code files
     * @return List of file paths
     */
    QStringList getCodeFiles() const;
    
    /**
     * Get the list of tape files
     * @return List of file paths
     */
    QStringList getTapeFiles() const;
    
    /**
     * Save the file lists to settings
     */
    void saveFileListsToSettings();
    
    /**
     * Load the file lists from settings
     */
    void loadFileListsFromSettings();
    
    /**
     * Update the active document indication
     * @param document The active document
     */
    void setActiveDocument(Document* document);

public slots:
    /**
     * Create a new code file
     */
    void createNewCodeFile();
    
    /**
     * Create a new tape file
     */
    void createNewTapeFile();
    
    /**
     * Open a code file
     */
    void openCodeFile();
    
    /**
     * Open a tape file
     */
    void openTapeFile();
    
    /**
     * Remove the selected file
     * @param isCodeFile True if it's a code file, false for tape file
     */
    void removeSelectedFile(bool isCodeFile);

signals:
    /**
     * Signal emitted when a code file is selected
     * @param path The file path
     */
    void codeFileSelected(const QString& path);
    
    /**
     * Signal emitted when a tape file is selected
     * @param path The file path
     */
    void tapeFileSelected(const QString& path);
    
    /**
     * Signal emitted when a new code document is created
     * @param document The new code document
     */
    void codeDocumentCreated(CodeDocument* document);
    
    /**
     * Signal emitted when a new tape document is created
     * @param document The new tape document
     */
    void tapeDocumentCreated(TapeDocument* document);

private slots:
    /**
     * Handle code file selection
     * @param item The selected item
     */
    void onCodeFileSelected(QListWidgetItem* item);
    
    /**
     * Handle tape file selection
     * @param item The selected item
     */
    void onTapeFileSelected(QListWidgetItem* item);
    
    /**
     * Handle code file double click (open the file)
     * @param item The double clicked item
     */
    void onCodeFileDoubleClicked(QListWidgetItem* item);
    
    /**
     * Handle tape file double click (open the file)
     * @param item The double clicked item
     */
    void onTapeFileDoubleClicked(QListWidgetItem* item);

private:
    QGroupBox* m_codeFilesGroup;
    QListWidget* m_codeFilesList;
    QPushButton* m_newCodeButton;
    QPushButton* m_openCodeButton;
    QPushButton* m_removeCodeButton;
    
    QGroupBox* m_tapeFilesGroup;
    QListWidget* m_tapeFilesList;
    QPushButton* m_newTapeButton;
    QPushButton* m_openTapeButton;
    QPushButton* m_removeTapeButton;
    
    // Map of file paths to their corresponding items
    QMap<QString, QListWidgetItem*> m_codeItems;
    QMap<QString, QListWidgetItem*> m_tapeItems;
    
    // Currently active document path
    QString m_activeDocumentPath;
    
    /**
     * Set up the UI
     */
    void setupUI();
    
    /**
     * Set up drag and drop
     */
    void setupDragDrop();
    
    /**
     * Create the code files accordion
     * @return The code files group box
     */
    QGroupBox* createCodeFilesAccordion();
    
    /**
     * Create the tape files accordion
     * @return The tape files group box
     */
    QGroupBox* createTapeFilesAccordion();
    
    /**
     * Update the selection in the file lists
     * @param path The currently active file path
     */
    void updateSelection(const QString& path);
    
    /**
     * Get the file icon based on extension
     * @param filePath The file path
     * @return The appropriate icon
     */
    QIcon getFileIcon(const QString& filePath);
    
    // Drag and drop implementation
protected:
    /**
     * Handle drag enter events
     * @param event The drag enter event
     */
    void dragEnterEvent(QDragEnterEvent* event) override;
    
    /**
     * Handle drop events
     * @param event The drop event
     */
    void dropEvent(QDropEvent* event) override;
};