#include "TapeControlWidget.h"
#include "../core/SessionManager.h"
#include "../model/Tape.h"
#include "../document/TapeDocument.h"
#include "TapeWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QDebug>

#include "StyleKit.h"

TapeControlWidget::TapeControlWidget(TapeWidget* tapeWidget, QWidget *parent)
    : QWidget(parent), m_tape(nullptr), m_tapeWidget(tapeWidget)
{
    setupUI();
    updateCurrentTapeLabel();

    // Connect to tape widget signals
    if (m_tapeWidget) {
        connect(m_tapeWidget, &TapeWidget::tapeModified, this, &TapeControlWidget::onTapeModified);
    }

    // Connect to SessionManager signals
    connect(&SessionManager::getInstance(), &SessionManager::activeTapeDocumentChanged,
            [this](TapeDocument* doc) {
                if (doc) {
                    onActiveTapeChanged(doc->getTape());
                } else {
                    onActiveTapeChanged(nullptr);
                }
            });

    // Initialize with current active tape
    updateForActiveTape();
}

TapeControlWidget::~TapeControlWidget()
{
}

void TapeControlWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Tape content input group
    QGroupBox* inputGroupBox = new QGroupBox(tr("Tape Content"));
    inputGroupBox->setStyleSheet(StyleKit::getGroupBoxStyle());
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroupBox);
    inputLayout->setSpacing(12);

    QHBoxLayout* tapeContentLayout = new QHBoxLayout();
    m_tapeContentEdit = new QLineEdit(this);
    m_tapeContentEdit->setPlaceholderText(tr("Enter tape content..."));
    m_tapeContentEdit->setStyleSheet(StyleKit::getInputStyle());
    tapeContentLayout->addWidget(new QLabel(tr("Content:")));
    tapeContentLayout->addWidget(m_tapeContentEdit);

    QHBoxLayout* headPositionLayout = new QHBoxLayout();
    m_initialHeadPositionSpin = new QSpinBox(this);
    m_initialHeadPositionSpin->setMinimum(0);
    m_initialHeadPositionSpin->setMaximum(999);
    m_initialHeadPositionSpin->setValue(0);
    m_initialHeadPositionSpin->setStyleSheet(StyleKit::getInputStyle());
    headPositionLayout->addWidget(new QLabel(tr("Initial Head Position:")));
    headPositionLayout->addWidget(m_initialHeadPositionSpin);
    headPositionLayout->addStretch();

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_setTapeButton = new QPushButton(tr("Set Tape"), this);
    m_resetTapeButton = new QPushButton(tr("Reset Tape"), this);

    // Apply button styles
    m_setTapeButton->setStyleSheet(StyleKit::getButtonStyle(true));  // Primary button
    m_resetTapeButton->setStyleSheet(StyleKit::getButtonStyle(false));  // Secondary button

    connect(m_setTapeButton, &QPushButton::clicked, this, &TapeControlWidget::setTapeContent);
    connect(m_resetTapeButton, &QPushButton::clicked, this, &TapeControlWidget::resetTape);
    buttonLayout->addWidget(m_setTapeButton);
    buttonLayout->addWidget(m_resetTapeButton);

    inputLayout->addLayout(tapeContentLayout);
    inputLayout->addLayout(headPositionLayout);
    inputLayout->addLayout(buttonLayout);

    // Tape controls group
    QGroupBox* controlGroupBox = new QGroupBox(tr("Tape Controls"));
    controlGroupBox->setStyleSheet(StyleKit::getGroupBoxStyle());
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroupBox);
    controlLayout->setSpacing(12);

    m_interactiveModeCheckbox = new QCheckBox(tr("Interactive Mode (click to move head, double-click to edit)"), this);
    m_interactiveModeCheckbox->setChecked(m_tapeWidget ? m_tapeWidget->isInteractiveMode() : true);
    connect(m_interactiveModeCheckbox, &QCheckBox::toggled, this, &TapeControlWidget::toggleInteractiveMode);
    controlLayout->addWidget(m_interactiveModeCheckbox);

    m_currentTapeLabel = new QLabel(this);
    m_currentTapeLabel->setAlignment(Qt::AlignCenter);
    m_currentTapeLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_currentTapeLabel->setMinimumHeight(36);
    m_currentTapeLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #F8FAFC;"  // Very light background
        "  border: 1px solid #CBD5E1;"  // Light gray border
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  color: #334155;"             // Dark slate
        "}"
    );
    controlLayout->addWidget(m_currentTapeLabel);

    QHBoxLayout* shiftLayout = new QHBoxLayout();
    m_shiftLeftButton = new QToolButton(this);
    m_shiftRightButton = new QToolButton(this);

    // Apply modern styling to tool buttons
    QString toolButtonStyle =
        "QToolButton {"
        "  background-color: #F1F5F9;"  // Very light gray
        "  border: 1px solid #CBD5E1;"  // Light gray border
        "  border-radius: 4px;"
        "  padding: 8px;"
        "}"
        "QToolButton:hover {"
        "  background-color: #E2E8F0;"  // Slightly darker gray
        "}"
        "QToolButton:pressed {"
        "  background-color: #CBD5E1;"  // Even darker gray
        "}";

    m_shiftLeftButton->setStyleSheet(toolButtonStyle);
    m_shiftRightButton->setStyleSheet(toolButtonStyle);

    m_shiftLeftButton->setIcon(QIcon::fromTheme("go-previous"));
    m_shiftRightButton->setIcon(QIcon::fromTheme("go-next"));
    m_shiftLeftButton->setIconSize(QSize(16, 16));
    m_shiftRightButton->setIconSize(QSize(16, 16));
    m_shiftLeftButton->setToolTip(tr("Shift Tape Left"));
    m_shiftRightButton->setToolTip(tr("Shift Tape Right"));
    connect(m_shiftLeftButton, &QToolButton::clicked, this, &TapeControlWidget::shiftLeft);
    connect(m_shiftRightButton, &QToolButton::clicked, this, &TapeControlWidget::shiftRight);
    shiftLayout->addStretch();
    shiftLayout->addWidget(m_shiftLeftButton);
    shiftLayout->addWidget(m_shiftRightButton);
    shiftLayout->addStretch();
    controlLayout->addLayout(shiftLayout);

    QHBoxLayout* zoomLayout = new QHBoxLayout();
    QPushButton* zoomInButton = new QPushButton(tr("+"), this);
    QPushButton* zoomOutButton = new QPushButton(tr("-"), this);
    QPushButton* resetZoomButton = new QPushButton(tr("Reset Zoom"), this);

    // Apply consistent button styling
    zoomInButton->setStyleSheet(StyleKit::getButtonStyle(false));
    zoomOutButton->setStyleSheet(StyleKit::getButtonStyle(false));
    resetZoomButton->setStyleSheet(StyleKit::getButtonStyle(false));

    if (m_tapeWidget) {
        connect(zoomInButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomIn);
        connect(zoomOutButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::zoomOut);
        connect(resetZoomButton, &QPushButton::clicked, m_tapeWidget, &TapeWidget::resetZoom);
    }
    zoomLayout->addWidget(new QLabel(tr("Zoom:")));
    zoomLayout->addWidget(zoomOutButton);
    zoomLayout->addWidget(resetZoomButton);
    zoomLayout->addWidget(zoomInButton);
    controlLayout->addLayout(zoomLayout);

    QHBoxLayout* speedLayout = new QHBoxLayout();
    m_speedLabel = new QLabel(tr("Speed (ms/step):"), this);
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(50, 1000);
    m_speedSlider->setValue(500);
    m_speedSlider->setTickPosition(QSlider::TicksBelow);
    m_speedSlider->setTickInterval(50);
    m_speedSlider->setStyleSheet(StyleKit::getSliderStyle());

    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int value) {
        emit speedChanged(value);
    });
    speedLayout->addWidget(m_speedLabel);
    speedLayout->addWidget(m_speedSlider);
    controlLayout->addLayout(speedLayout);

    mainLayout->addWidget(inputGroupBox);
    mainLayout->addWidget(controlGroupBox);
    mainLayout->addStretch();

    // Set initial enabled state
    bool hasActiveTape = (getActiveTape() != nullptr);
    m_setTapeButton->setEnabled(hasActiveTape);
    m_resetTapeButton->setEnabled(hasActiveTape);
    m_shiftLeftButton->setEnabled(hasActiveTape);
    m_shiftRightButton->setEnabled(hasActiveTape);
}

void TapeControlWidget::updateForActiveTape()
{
    m_tape = getActiveTape();

    // Update UI state
    bool hasTape = (m_tape != nullptr);
    m_setTapeButton->setEnabled(hasTape);
    m_resetTapeButton->setEnabled(hasTape);
    m_shiftLeftButton->setEnabled(hasTape);
    m_shiftRightButton->setEnabled(hasTape);
    m_tapeContentEdit->setEnabled(hasTape);
    m_initialHeadPositionSpin->setEnabled(hasTape);

    // Update tape widget
    if (m_tapeWidget) {
        m_tapeWidget->setTape(m_tape);
        m_tapeWidget->updateTapeDisplay();
    }

    // Update content display
    if (m_tape) {
        m_tapeContentEdit->setText(QString::fromStdString(m_tape->getCurrentContent()));
        m_initialHeadPositionSpin->setValue(m_tape->getHeadPosition());
    } else {
        m_tapeContentEdit->clear();
        m_initialHeadPositionSpin->setValue(0);
    }

    updateCurrentTapeLabel();
}

void TapeControlWidget::toggleInteractiveMode(bool enabled)
{
    if (m_tapeWidget) {
        m_tapeWidget->setInteractiveMode(enabled);
    }
}

void TapeControlWidget::onTapeModified()
{
    updateCurrentTapeLabel();
    emit tapeContentChanged();
}

void TapeControlWidget::resetTape()
{
    m_tape = getActiveTape();
    if (m_tape) {
        m_tape->reset();
        if (m_tapeWidget) {
            m_tapeWidget->updateTapeDisplay();
        }
        updateCurrentTapeLabel();
        emit tapeContentChanged();
    }
}

void TapeControlWidget::setTapeContent()
{
    m_tape = getActiveTape();
    if (m_tape) {
        QString content = m_tapeContentEdit->text();
        int headPos = m_initialHeadPositionSpin->value();

        m_tape->reset();
        m_tape->setInitialContent(content.toStdString());
        m_tape->setHeadPosition(headPos);

        if (m_tapeWidget) {
            m_tapeWidget->updateTapeDisplay();
        }
        updateCurrentTapeLabel();
        emit tapeContentChanged();

        // Also update the tape document
        TapeDocument* doc = dynamic_cast<TapeDocument*>(SessionManager::getInstance().getActiveTapeDocument());
        if (doc) {
            doc->setInitialContent(content.toStdString());
            doc->setInitialHeadPosition(headPos);
        }
    }
}

void TapeControlWidget::shiftLeft()
{
    m_tape = getActiveTape();
    if (m_tape) {
        m_tape->moveLeft();
        if (m_tapeWidget) {
            m_tapeWidget->animateHeadMovement(false);
        }
        updateCurrentTapeLabel();
    }
}

void TapeControlWidget::shiftRight()
{
    m_tape = getActiveTape();
    if (m_tape) {
        m_tape->moveRight();
        if (m_tapeWidget) {
            m_tapeWidget->animateHeadMovement(true);
        }
        updateCurrentTapeLabel();
    }
}

void TapeControlWidget::updateCurrentTapeLabel()
{
    m_tape = getActiveTape();
    if (m_tape) {
        QString content = QString::fromStdString(m_tape->getCurrentContent(20));
        int headPos = m_tape->getHeadPosition();

        QString displayText = tr("Position: %1,  Content: %2")
            .arg(headPos)
            .arg(content);

        m_currentTapeLabel->setText(displayText);
    } else {
        m_currentTapeLabel->setText(tr("No active tape"));
    }
}

void TapeControlWidget::onActiveTapeChanged(Tape* tape)
{
    m_tape = tape;
    updateForActiveTape();
}

Tape* TapeControlWidget::getActiveTape() const
{
    TapeDocument* doc = dynamic_cast<TapeDocument*>(SessionManager::getInstance().getActiveTapeDocument());
    return doc ? doc->getTape() : nullptr;
}