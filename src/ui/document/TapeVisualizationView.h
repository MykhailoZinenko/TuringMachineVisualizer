#pragma once

#include "DocumentView.h"
#include <memory>

class TapeDocument;
class TapeWidget;
class QPushButton;
class QLineEdit;
class QSpinBox;
class QLabel;
class QTimer;

/**
 * View for visualizing and running a tape
 */
class TapeVisualizationView : public DocumentView
{
    Q_OBJECT

public:
    TapeVisualizationView(TapeDocument* document, QWidget* parent = nullptr);
    ~TapeVisualizationView() override;

    // Update view from document
    void updateFromDocument() override;

    private slots:
        void setTapeContent();
    void resetTape();
    void runSimulation();
    void pauseSimulation();
    void stepForward();
    void stepBackward();
    void onTapeContentChanged();
    void onSimulationSpeed(int value);
    void onSimulationTimerTick();
    void onExecutionStateChanged();

private:
    TapeDocument* m_tapeDocument;

    // UI components
    TapeWidget* m_tapeWidget;
    QLineEdit* m_contentEdit;
    QSpinBox* m_headPositionSpin;
    QPushButton* m_setButton;
    QPushButton* m_resetButton;
    QPushButton* m_runButton;
    QPushButton* m_pauseButton;
    QPushButton* m_stepForwardButton;
    QPushButton* m_stepBackwardButton;
    QLabel* m_statusLabel;
    QTimer* m_simulationTimer;
    int m_simulationSpeed;

    void setupUI();
    void updateSimulationControls();
    void setStatusMessage(const QString& message, bool isError = false);
};