#pragma once

#include <QObject>
#include <memory>

// Forward declarations
class CodeDocument;
class TapeDocument;
class TuringMachine;

/**
 * Singleton class that manages the active code and tape documents
 * and their connections in the current session.
 */
class SessionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Get the singleton instance of the SessionManager.
     */
    static SessionManager& getInstance();

    /**
     * Set the active code document.
     * @param document The code document to set as active.
     */
    void setActiveCodeDocument(CodeDocument* document);

    /**
     * Set the active tape document.
     * @param document The tape document to set as active.
     */
    void setActiveTapeDocument(TapeDocument* document);

    /**
     * Get the currently active code document.
     * @return Pointer to the active code document, or nullptr if none.
     */
    CodeDocument* getActiveCodeDocument() const;

    /**
     * Get the currently active tape document.
     * @return Pointer to the active tape document, or nullptr if none.
     */
    TapeDocument* getActiveTapeDocument() const;

    /**
     * Get the active Turing machine from the current code document.
     * @return Pointer to the active machine, or nullptr if none.
     */
    TuringMachine* getActiveMachine() const;

signals:
    /**
     * Signal emitted when the active code document changes.
     * @param document The new active code document.
     */
    void activeCodeDocumentChanged(CodeDocument* document);

    /**
     * Signal emitted when the active tape document changes.
     * @param document The new active tape document.
     */
    void activeTapeDocumentChanged(TapeDocument* document);

    /**
     * Signal emitted when the active machine is updated (e.g. after code save)
     * @param machine The updated machine.
     */
    void activeMachineUpdated(TuringMachine* machine);

private:
    // Private constructor for singleton
    SessionManager();
    ~SessionManager();

    // Prevent copying
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    // Member variables
    CodeDocument* m_activeCodeDocument;
    TapeDocument* m_activeTapeDocument;
};