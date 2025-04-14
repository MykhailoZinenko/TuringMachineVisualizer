#pragma once

#include <QWidget>
#include <string>

// Forward declarations
class QLineEdit;
class QPushButton;
class QToolButton;
class QLabel;
class QSpinBox;
class QSlider;
class QCheckBox;
class Tape;
class TapeWidget;

class TapeControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TapeControlWidget(TapeWidget* tapeWidget, QWidget *parent = nullptr);
    ~TapeControlWidget();

    // Update the widget when the active tape changes
    void updateForActiveTape();

    signals:
        void tapeContentChanged();
    void speedChanged(int speed);

    public slots:
        void resetTape();
    void setTapeContent();
    void shiftLeft();
    void shiftRight();
    void toggleInteractiveMode(bool enabled);
    void onTapeModified();
    void onActiveTapeChanged(Tape* tape);

private:
    // Data
    Tape* m_tape;  // Non-owning reference to active tape
    TapeWidget* m_tapeWidget;

    // UI elements
    QLineEdit* m_tapeContentEdit;
    QSpinBox* m_initialHeadPositionSpin;
    QPushButton* m_setTapeButton;
    QPushButton* m_resetTapeButton;
    QToolButton* m_shiftLeftButton;
    QToolButton* m_shiftRightButton;
    QLabel* m_currentTapeLabel;
    QCheckBox* m_interactiveModeCheckbox;
    QSlider* m_speedSlider;
    QLabel* m_speedLabel;

    // Setup methods
    void setupUI();
    void updateCurrentTapeLabel();

    // Helper to get active tape from SessionManager
    Tape* getActiveTape() const;
};