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
    explicit TapeControlWidget(Tape* tape, TapeWidget* tapeWidget, QWidget *parent = nullptr);
    void setTape(Tape* tape);

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

private:
    // Data
    Tape* m_tape;
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
};