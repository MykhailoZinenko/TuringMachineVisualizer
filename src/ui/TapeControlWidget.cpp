#include "TapeControlWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QIcon>

TapeControlWidget::TapeControlWidget(Tape* tape, TapeWidget* tapeWidget, QWidget *parent)
    : QWidget(parent), m_tape(tape), m_tapeWidget(tapeWidget)
{
    setupUI();
    updateCurrentTapeLabel();
}

void TapeControlWidget::setTape(Tape* tape)
{
    m_tape = tape;
    resetTape(); // Reset immediately to sync with the new tape
}

void TapeControlWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tape content input area
    QGroupBox* inputGroupBox = new QGroupBox(tr("Tape Content"));
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroupBox);
    
    // Tape content input
    QHBoxLayout* tapeContentLayout = new QHBoxLayout();
    m_tapeContentEdit = new QLineEdit(this);
    m_tapeContentEdit->setPlaceholderText(tr("Enter tape content..."));
    tapeContentLayout->addWidget(new QLabel(tr("Content:")));
    tapeContentLayout->addWidget(m_tapeContentEdit);
    
    // Head position input
    QHBoxLayout* headPositionLayout = new QHBoxLayout();
    m_initialHeadPositionSpin = new QSpinBox(this);
    m_initialHeadPositionSpin->setMinimum(0);
    m_initialHeadPositionSpin->setMaximum(999);
    m_initialHeadPositionSpin->setValue(0);
    headPositionLayout->addWidget(new QLabel(tr("Initial Head Position:")));
    headPositionLayout->addWidget(m_initialHeadPositionSpin);
    headPositionLayout->addStretch();
    
    // Set and reset buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_setTapeButton = new QPushButton(tr("Set Tape"), this);
    m_resetTapeButton = new QPushButton(tr("Reset Tape"), this);
    connect(m_setTapeButton, &QPushButton::clicked, this, &TapeControlWidget::setTapeContent);
    connect(m_resetTapeButton, &QPushButton::clicked, this, &TapeControlWidget::resetTape);
    buttonLayout->addWidget(m_setTapeButton);
    buttonLayout->addWidget(m_resetTapeButton);
    
    // Add layouts to input group
    inputLayout->addLayout(tapeContentLayout);
    inputLayout->addLayout(headPositionLayout);
    inputLayout->addLayout(buttonLayout);
    
    // Create tape control area
    QGroupBox* controlGroupBox = new QGroupBox(tr("Tape Controls"));
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroupBox);
    
    // Current tape content display
    m_currentTapeLabel = new QLabel(this);
    m_currentTapeLabel->setAlignment(Qt::AlignCenter);
    m_currentTapeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_currentTapeLabel->setMinimumHeight(30);
    controlLayout->addWidget(m_currentTapeLabel);
    
    // Shift buttons
    QHBoxLayout* shiftLayout = new QHBoxLayout();
    m_shiftLeftButton = new QToolButton(this);
    m_shiftRightButton = new QToolButton(this);
    m_shiftLeftButton->setIcon(QIcon::fromTheme("go-previous"));
    m_shiftRightButton->setIcon(QIcon::fromTheme("go-next"));
    m_shiftLeftButton->setToolTip(tr("Shift Tape Left"));
    m_shiftRightButton->setToolTip(tr("Shift Tape Right"));
    connect(m_shiftLeftButton, &QToolButton::clicked, this, &TapeControlWidget::shiftLeft);
    connect(m_shiftRightButton, &QToolButton::clicked, this, &TapeControlWidget::shiftRight);
    shiftLayout->addStretch();
    shiftLayout->addWidget(m_shiftLeftButton);
    shiftLayout->addWidget(m_shiftRightButton);
    shiftLayout->addStretch();
    controlLayout->addLayout(shiftLayout);
    
    // Zoom controls
    QHBoxLayout* zoomLayout = new QHBoxLayout();
    QPushButton* zoomInButton = new QPushButton(tr("+"), this);
    QPushButton* zoomOutButton = new QPushButton(tr("-"), this);
    QPushButton* resetZoomButton = new QPushButton(tr("Reset Zoom"), this);
    connect(zoomInButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomIn);
    connect(zoomOutButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomOut);
    connect(resetZoomButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::resetZoom);
    zoomLayout->addWidget(new QLabel(tr("Zoom:")));
    zoomLayout->addWidget(zoomOutButton);
    zoomLayout->addWidget(resetZoomButton);
    zoomLayout->addWidget(zoomInButton);
    controlLayout->addLayout(zoomLayout);

    QHBoxLayout* speedLayout = new QHBoxLayout();
    m_speedLabel = new QLabel(tr("Speed (ms/step):"), this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(50, 1000); // 50ms to 1000ms
    m_speedSlider->setValue(500);      // Default 500ms
    m_speedSlider->setTickPosition(QSlider::TicksBelow);
    m_speedSlider->setTickInterval(50);
    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int value) {
        emit speedChanged(value);
    });
    speedLayout->addWidget(m_speedLabel);
    speedLayout->addWidget(m_speedSlider);
    controlLayout->addLayout(speedLayout);
    
    // Add groups to main layout
    mainLayout->addWidget(inputGroupBox);
    mainLayout->addWidget(controlGroupBox);
    mainLayout->addStretch();
}

void TapeControlWidget::resetTape()
{
    if (m_tape) {
        m_tape->reset();
        m_tapeWidget->updateTapeDisplay();
        updateCurrentTapeLabel();
        emit tapeContentChanged();
    }
}

void TapeControlWidget::setTapeContent()
{
    if (m_tape) {
        QString content = m_tapeContentEdit->text();
        int headPos = m_initialHeadPositionSpin->value();
        
        m_tape->reset();
        m_tape->setInitialContent(content.toStdString());
        m_tape->setHeadPosition(headPos);
        
        m_tapeWidget->updateTapeDisplay();
        updateCurrentTapeLabel();
        emit tapeContentChanged();
    }
}

void TapeControlWidget::shiftLeft()
{
    if (m_tape) {
        // Move head left (showing animation)
        m_tape->moveLeft();
        m_tapeWidget->animateHeadMovement(false);
        updateCurrentTapeLabel();
    }
}

void TapeControlWidget::shiftRight()
{
    if (m_tape) {
        // Move head right (showing animation)
        m_tape->moveRight();
        m_tapeWidget->animateHeadMovement(true);
        updateCurrentTapeLabel();
    }
}

void TapeControlWidget::updateCurrentTapeLabel()
{
    if (m_tape) {
        QString content = QString::fromStdString(m_tape->getCurrentContent(20));
        int headPos = m_tape->getHeadPosition();
        
        QString displayText = tr("Position: %1,  Content: %2")
            .arg(headPos)
            .arg(content);
            
        m_currentTapeLabel->setText(displayText);
    }
}