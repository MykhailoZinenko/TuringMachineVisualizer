#pragma once

#include "Document.h"
#include <memory>
#include <string>
#include <QObject>
#include <map>

// Forward declarations
class Tape;
class TuringMachine;

/**
 * Document representing a tape for visualization and simulation (.tmt file)
 */
class TapeDocument : public Document
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param name The display name of the document
     * @param filePath The file path, or empty if not yet saved
     */
    TapeDocument(const std::string& name = "Untitled", const std::string& filePath = "");

    /**
     * Destructor
     */
    ~TapeDocument() override;

    /**
     * Get the tape associated with this document
     * @return Pointer to the tape
     */
    Tape* getTape() const { return m_tape.get(); }

    /**
     * Set the initial content of the tape
     * @param content The content to set
     */
    void setInitialContent(const std::string& content);

    /**
     * Set the initial head position
     * @param position The head position
     */
    void setInitialHeadPosition(int position);

    /**
     * Get the initial head position
     * @return The head position
     */
    int getInitialHeadPosition() const { return m_initialHeadPosition; }

    /**
     * Execute a single step of the Turing machine
     * @param isManualStep Whether this is a manual step (vs. automated run)
     * @return True if step was successful, false if halted or error
     */
    bool step(bool isManualStep = false);

    /**
     * Reset the tape to its initial state
     */
    void reset();

    /**
     * Begin running the simulation
     */
    void run();

    /**
     * Pause the running simulation
     */
    void pause();

    /**
     * Stop the simulation (reset machine state but leave tape content)
     */
    void stop();

    /**
     * Check if we can step backward
     * @return True if we can step backward, false otherwise
     */
    bool canStepBackward() const;

    /**
     * Execute a step backward
     * @return True if successful, false otherwise
     */
    bool stepBackward();

    /**
     * Load document from a file
     * @param filePath The file path to load from
     * @return True if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filePath) override;

signals:
    /**
     * Signal emitted when the tape content changes
     */
    void tapeContentChanged();

    /**
     * Signal emitted when the execution state changes
     */
    void executionStateChanged();

public slots:
    /**
     * Handle active machine updates
     * @param machine The updated machine
     */
    void onActiveMachineUpdated(TuringMachine* machine);

protected:
    /**
     * Save to a file
     * @param filePath The file path to save to
     * @return True if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filePath) override;

private:
    std::unique_ptr<Tape> m_tape;
    std::string m_initialContent;
    int m_initialHeadPosition;

    /**
     * Get the active machine from the SessionManager
     * @return Pointer to the active machine, or nullptr if none
     */
    TuringMachine* getActiveMachine() const;
};