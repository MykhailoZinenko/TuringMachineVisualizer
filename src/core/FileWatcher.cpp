#include "FileWatcher.h"
#include <QFileInfo>
#include <QDebug>

FileWatcher& FileWatcher::getInstance()
{
    static FileWatcher instance;
    return instance;
}

FileWatcher::FileWatcher()
    : m_debounceMs(300) // 300ms debounce time
{
    // Set up debounce timer
    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout, 
            this, &FileWatcher::processPendingChanges);

    // Connect file watcher signals
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileWatcher::onFileChanged);
}

FileWatcher::~FileWatcher()
{
}

bool FileWatcher::watchFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qWarning() << "Cannot watch non-existent file:" << filePath;
        return false;
    }

    // Check if already watching
    if (m_fileWatcher.files().contains(filePath)) {
        qDebug() << "Already watching file:" << filePath;
        return true;
    }

    // Add to watcher
    bool success = m_fileWatcher.addPath(filePath);
    if (success) {
        qDebug() << "Now watching file:" << filePath;
    } else {
        qWarning() << "Failed to watch file:" << filePath;
    }
    
    return success;
}

void FileWatcher::unwatchFile(const QString& filePath)
{
    if (m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.removePath(filePath);
        qDebug() << "Stopped watching file:" << filePath;
    }
    
    // Also remove from pending changes if present
    m_pendingChanges.remove(filePath);
}

bool FileWatcher::isWatching(const QString& filePath) const
{
    return m_fileWatcher.files().contains(filePath);
}

void FileWatcher::onFileChanged(const QString& filePath)
{
    // Check if file still exists (sometimes we get notifications when file is deleted)
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qDebug() << "Watched file was deleted:" << filePath;
        unwatchFile(filePath);
        return;
    }
    
    // Add to pending changes
    m_pendingChanges[filePath] = QDateTime::currentDateTime();
    
    // Start or restart debounce timer
    if (m_debounceTimer.isActive()) {
        m_debounceTimer.stop();
    }
    m_debounceTimer.start(m_debounceMs);
    
    // If file was replaced (common with editors that use temp files),
    // we need to add it to the watcher again
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

void FileWatcher::processPendingChanges()
{
    QDateTime now = QDateTime::currentDateTime();
    QStringList processedFiles;
    
    // Process all pending changes
    QMapIterator<QString, QDateTime> it(m_pendingChanges);
    while (it.hasNext()) {
        it.next();
        
        // Check if enough time has passed since the last change
        int msSinceChange = it.value().msecsTo(now);
        if (msSinceChange >= m_debounceMs) {
            processedFiles.append(it.key());
            emit fileChanged(it.key());
        }
    }
    
    // Remove processed files from pending list
    for (const QString& file : processedFiles) {
        m_pendingChanges.remove(file);
    }
    
    // If there are still pending changes, restart the timer
    if (!m_pendingChanges.isEmpty()) {
        m_debounceTimer.start(m_debounceMs);
    }
}