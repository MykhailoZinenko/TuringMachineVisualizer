#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <QString>

/**
 * Class that watches files for changes and emits signals when they are modified.
 * Includes debouncing to prevent multiple signals when saving files.
 */
class FileWatcher : public QObject
{
    Q_OBJECT

public:
    /**
     * Get the singleton instance of the FileWatcher.
     */
    static FileWatcher& getInstance();

    /**
     * Start watching a file for changes.
     * @param filePath The path to the file to watch.
     * @return True if file was added to watch list, false otherwise.
     */
    bool watchFile(const QString& filePath);

    /**
     * Stop watching a file.
     * @param filePath The path to the file to stop watching.
     */
    void unwatchFile(const QString& filePath);

    /**
     * Check if a file is currently being watched.
     * @param filePath The path to check.
     * @return True if the file is being watched, false otherwise.
     */
    bool isWatching(const QString& filePath) const;

    signals:
        /**
         * Signal emitted when a watched file is modified.
         * @param filePath The path to the modified file.
         */
        void fileChanged(const QString& filePath);

    private slots:
        /**
         * Handle file changed notifications from the QFileSystemWatcher.
         * Implements debouncing.
         * @param filePath The path to the changed file.
         */
        void onFileChanged(const QString& filePath);

    /**
     * Process files that have been modified after the debounce period.
     */
    void processPendingChanges();

private:
    // Private constructor for singleton
    FileWatcher();
    ~FileWatcher();

    // Prevent copying
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    // Member variables
    QFileSystemWatcher m_fileWatcher;
    QTimer m_debounceTimer;
    QMap<QString, QDateTime> m_pendingChanges;
    int m_debounceMs;
};