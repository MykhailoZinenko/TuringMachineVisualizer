#ifndef TAPECONTROLWIDGET_H
#define TAPECONTROLWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include "../model/Tape.h"
#include "TapeWidget.h"

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
    Tape* m_tape;
    TapeWidget* m_tapeWidget;

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

    void setupUI();
    void updateCurrentTapeLabel();
};

#endif // TAPECONTROLWIDGET_H