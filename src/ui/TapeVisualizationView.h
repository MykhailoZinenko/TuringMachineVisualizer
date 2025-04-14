#pragma once

#include <QWidget>
#include <memory>

// Forward declarations
class TapeDocument;
class TapeWidget;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QToolButton;
class QSlider;
class QTimer;
class TuringMachine;

/**
 * View for visualizing and controlling a Turing machine tape
 */
class TapeVisualizationView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param document The tape document to visualize
     * @param parent The parent widget
     */
    explicit TapeVisualizationView(TapeDocument* document, QWidget* parent = nullptr);

    /**
     * Destructor
     */
    ~TapeVisualizationView();

    /**
     * Get the document being visualized
     * @return The document
     */
    TapeDocument* getDocument() const { return m_document; }

    /**
     * Update the view from the document
     */
    void updateFromDocument();

signals:
    /**
     * Signal emitted when the view's content is modified
     */
    void viewModified();

    /**
     * Signal emitted when the view is saved
     */
    void viewSaved();

private slots:
    /**
     * Handle tape content change
     */
    void onTapeContentChanged();

    /**
     * Handle execution state change
     */
    void onExecutionStateChanged();

    /**
     * Handle document save
     * @param filePath The path where the document was saved
     */
    void onDocumentSaved(const std::string& filePath);

    /**
     * Set the tape content from the UI
     */
    void setTapeContent();

    /**
     * Reset the tape to initial state
     */
    void resetTape();

    /**
     * Run the simulation
     */
    void runSimulation();

    /**
     * Pause the simulation
     */
    void pauseSimulation();

    /**
     * Stop the simulation
     */
    void stopSimulation();

    /**
     * Execute a single step forward
     */
    void stepForward();

    /**
     * Execute a single step backward
     */
    void stepBackward();

    /**
     * Handle simulation speed change
     * @param value The new speed value
     */
    void onSimulationSpeedChanged(int value);

    /**
     * Handle simulation timer tick
     */
    void onSimulationTimerTick();

    /**
     * Handle active machine update
     */
    void onActiveMachineUpdated(TuringMachine* machine);

    /**
     * Handle tape interaction
     */
    void onTapeModified();

    /**
     * Handle tape cell value change
     * @param position The cell position
     * @param newValue The new cell value
     */
    void onCellValueChanged(int position, const std::string& newValue);

    /**
     * Handle tape head position change
     * @param newPosition The new head position
     */
    void onHeadPositionChanged(int newPosition);

private:
    // Document being visualized
    TapeDocument* m_document;

    // UI components
    TapeWidget* m_tapeWidget;
    QLineEdit* m_contentEdit;
    QSpinBox* m_headPositionSpin;
    QPushButton* m_setButton;
    QPushButton* m_resetButton;
    QPushButton* m_runButton;
    QPushButton* m_pauseButton;
    QPushButton* m_stopButton;
    QPushButton* m_stepForwardButton;
    QPushButton* m_stepBackwardButton;
    QLabel* m_statusLabel;
    QSlider* m_speedSlider;
    QLabel* m_speedLabel;

    // Simulation timer
    QTimer* m_simulationTimer;
    int m_simulationSpeed;

    /**
     * Set up the UI
     */
    void setupUI();

    /**
     * Update the simulation controls based on current state
     */
    void updateSimulationControls();

    /**
     * Set the status message
     * @param message The message to display
     * @param isError Whether the message is an error
     */
    void setStatusMessage(const QString& message, bool isError = false);
};