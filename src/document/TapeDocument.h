#pragma once

#include "Document.h"
#include <memory>
#include <string>

class Tape;

/**
 * Document representing a tape for visualization and simulation
 */
class TapeDocument : public Document
{
    Q_OBJECT

public:
    TapeDocument(Project* project, const std::string& id, const std::string& name = "Tape");
    ~TapeDocument() override;

    // Tape access
    Tape* getTape() const { return m_tape.get(); }

    // Tape configuration
    void setInitialContent(const std::string& content);
    void setInitialHeadPosition(int position);

    // Execution methods
    bool step();
    void reset();
    void run();
    void pause();
    bool canStepBackward() const;
    bool stepBackward();

    signals:
        void tapeContentChanged();
    void executionStateChanged();

private:
    std::unique_ptr<Tape> m_tape;
    std::string m_initialContent;
    int m_initialHeadPosition;
};