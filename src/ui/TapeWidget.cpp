#include "TapeWidget.h"

#include <iostream>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDebug>

TapeWidget::TapeWidget(QWidget *parent)
    : QWidget(parent), m_tape(nullptr), m_visibleCells(15), m_cellSize(40),
      m_leftmostCell(0), m_headAnimOffset(0), m_headAnimation(0.0),
      m_interactiveMode(true) // Default to interactive mode
{
    // Setup widget
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    // Enable context menu and mouse tracking
    setContextMenuPolicy(Qt::DefaultContextMenu);
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TapeWidget::updateTapeDisplay);
    m_updateTimer->start(100); // Update every 100ms
    
    // Setup animation
    m_headAnimationObj = new QPropertyAnimation(this, "headAnimation");
    m_headAnimationObj->setDuration(300); // 300ms animation
    m_headAnimationObj->setEasingCurve(QEasingCurve::OutCubic);
}

void TapeWidget::setInteractiveMode(bool enabled)
{
    m_interactiveMode = enabled;
}

void TapeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    // Convert click position to cell index
    int cellIndex = xToCell(event->pos().x());

    // Edit the cell value
    editCellValue(cellIndex);

    QWidget::mouseDoubleClickEvent(event);
}

void TapeWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // Convert click position to cell index
        int cellIndex = xToCell(event->pos().x());

        // Move the head to the clicked cell
        moveHeadToCell(cellIndex);
    }

    QWidget::mousePressEvent(event);
}

void TapeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::contextMenuEvent(event);
        return;
    }

    // Convert position to cell index
    int cellIndex = xToCell(event->pos().x());

    // Create context menu
    QMenu contextMenu(this);

    QAction* editAction = contextMenu.addAction(tr("Edit Cell Value"));
    QAction* moveHeadAction = contextMenu.addAction(tr("Move Head Here"));
    QAction* clearAction = contextMenu.addAction(tr("Clear Cell"));

    // Add separator for navigation options
    contextMenu.addSeparator();
    QAction* centerAction = contextMenu.addAction(tr("Center View on Head"));
    QAction* resetZoomAction = contextMenu.addAction(tr("Reset Zoom"));

    // Execute the menu and get the selected action
    QAction* selectedAction = contextMenu.exec(event->globalPos());

    if (selectedAction == editAction) {
        editCellValue(cellIndex);
    } else if (selectedAction == moveHeadAction) {
        moveHeadToCell(cellIndex);
    } else if (selectedAction == clearAction) {
        if (m_tape) {
            int originalHeadPos = m_tape->getHeadPosition();
            m_tape->setHeadPosition(cellIndex);
            m_tape->write(m_tape->getBlankSymbol());
            m_tape->setHeadPosition(originalHeadPos); // Restore original head position
            updateTapeDisplay();
            emit cellValueChanged(cellIndex, m_tape->getBlankSymbol());
            emit tapeModified();
        }
    } else if (selectedAction == centerAction) {
        centerHeadPosition();
        updateTapeDisplay();
    } else if (selectedAction == resetZoomAction) {
        resetZoom();
    }
}

void TapeWidget::editCellValue(int cellIndex)
{
    if (!m_tape) return;

    // Determine current value at the cell
    int originalHeadPos = m_tape->getHeadPosition();
    m_tape->setHeadPosition(cellIndex);
    char currentSymbol = m_tape->read();

    // Create string for current value (handling blank symbol)
    QString currentValue = (currentSymbol == m_tape->getBlankSymbol()) ? "" : QString(currentSymbol);

    // Show input dialog
    bool ok;
    QString newValue = QInputDialog::getText(this, tr("Edit Cell Value"),
                                           tr("Enter new cell value (empty for blank):"),
                                           QLineEdit::Normal,
                                           currentValue, &ok);

    // If user clicked OK and provided a value
    if (ok) {
        char newSymbol;
        if (newValue.isEmpty()) {
            newSymbol = m_tape->getBlankSymbol();
        } else {
            newSymbol = newValue.at(0).toLatin1(); // Just take the first character
        }

        // Write the new value
        m_tape->write(newSymbol);

        // Restore original head position
        m_tape->setHeadPosition(originalHeadPos);

        // Update display
        updateTapeDisplay();

        // Notify about the change
        emit cellValueChanged(cellIndex, newSymbol);
        emit tapeModified();
    } else {
        // Restore original head position
        m_tape->setHeadPosition(originalHeadPos);
    }
}

void TapeWidget::moveHeadToCell(int cellIndex)
{
    if (!m_tape) return;

    // Get current head position
    int oldPosition = m_tape->getHeadPosition();

    // Only update if different from current position
    if (cellIndex != oldPosition) {
        // Determine if moving right or left
        bool movingRight = cellIndex > oldPosition;

        // Set the new head position
        m_tape->setHeadPosition(cellIndex);

        // Animate the head movement
        animateHeadMovement(movingRight);

        // Notify about the change
        emit headPositionChanged(cellIndex);
        emit tapeModified();
    }
}

TapeWidget::~TapeWidget()
{
    // Clean up
    m_updateTimer->stop();
    delete m_headAnimationObj;
}

void TapeWidget::setTape(Tape* tape)
{
    m_tape = tape;
    centerHeadPosition();
    updateTapeDisplay();
}

void TapeWidget::updateTapeDisplay()
{
    if (m_tape) {
        ensureHeadVisible();
        update(); // Request a repaint
    }
}

// Add a new method to check and ensure the head is visible
void TapeWidget::ensureHeadVisible()
{
    if (!m_tape) return;

    int headPosition = m_tape->getHeadPosition();

    // Check if the head is outside the visible range
    if (headPosition < m_leftmostCell || headPosition >= m_leftmostCell + m_visibleCells) {
        // Head is outside visible range, adjust view
        // We center it to provide context on both sides
        centerHeadPosition();
    }
}

// Modify the setHeadAnimation method to better handle head visibility after animation
void TapeWidget::setHeadAnimation(qreal value)
{
    m_headAnimation = value;
    update(); // Request repaint when animation value changes

    // When animation completes, ensure cell is visible
    if (value >= 0.99) {
        m_headAnimation = 0.0;
        m_headAnimOffset = 0;
        ensureHeadVisible(); // Use our new method
    }
}

void TapeWidget::animateHeadMovement(bool moveRight)
{
    if (!m_headAnimationObj->state() == QPropertyAnimation::Running) {
        m_headAnimation = 0.0;
        m_headAnimationObj->setStartValue(0.0);
        m_headAnimationObj->setEndValue(1.0);
        m_headAnimOffset = moveRight ? 1 : -1;
        m_headAnimationObj->start();
    }
}

void TapeWidget::onStepExecuted()
{
    if (!m_tape) return;

    // Instead of always centering (which can be disorienting),
    // only move the view if the head would be outside the visible range
    ensureHeadVisible();

    updateTapeDisplay();
}

void TapeWidget::zoomIn()
{
    if (m_cellSize < 100) {
        m_cellSize += 5;
        updateCellSize();
        update();
    }
}

void TapeWidget::zoomOut()
{
    if (m_cellSize > 20) {
        m_cellSize -= 5;
        updateCellSize();
        update();
    }
}

void TapeWidget::resetZoom()
{
    m_cellSize = 40;
    updateCellSize();
    update();
}

void TapeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    if (!m_tape) return;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), QColor(245, 245, 245));
    
    // Draw grid
    drawGrid(painter);
    
    // Get cell range to display
    int start = m_leftmostCell;
    int end = start + m_visibleCells;

    // Draw cells in left-to-right order (matching their logical indices)
    for (int cellIndex = start; cellIndex < end; ++cellIndex) {
        QRect cellRect = getCellRect(cellIndex);

        if (cellRect.intersects(event->rect())) {
            // Find the symbol at this position directly from the tape
            char symbol = '_';  // Default blank symbol
            auto cells = m_tape->getVisiblePortion(cellIndex, 1);
            if (!cells.empty()) {
                symbol = cells[0].second;
            }

            // Draw cell with its content
            drawCell(painter, cellIndex, cellRect, symbol);
        }
    }

    // Draw read/write head
    drawHead(painter);
}

void TapeWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void TapeWidget::wheelEvent(QWheelEvent *event)
{
    // Zoom in/out with Ctrl+wheel
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    } 
    // Scroll tape with wheel
    else {
        if (event->angleDelta().y() > 0) {
            m_leftmostCell--;
        } else {
            m_leftmostCell++;
        }

        // After manual scrolling, ensure head remains visible if it was visible before
        if (m_tape) {
            int headPosition = m_tape->getHeadPosition();
            // Only enforce visibility if the head was already visible
            // This allows the user to explore the tape without the view jumping back
            bool wasHeadVisible = (headPosition >= m_leftmostCell &&
                                  headPosition < m_leftmostCell + m_visibleCells);

            if (wasHeadVisible) {
                ensureHeadVisible();
            }
        }

        updateTapeDisplay();
    }
    
    event->accept();
}

void TapeWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateCellSize();
}

int TapeWidget::xToCell(int x) const
{
    return m_leftmostCell + x / m_cellSize;
}

QRect TapeWidget::getCellRect(int cellIndex) const
{
    int x = (cellIndex - m_leftmostCell) * m_cellSize;
    return QRect(x, 0, m_cellSize, height());
}

void TapeWidget::centerHeadPosition()
{
    if (!m_tape) return;
    
    // Calculate leftmost cell to center the head position
    int headPosition = m_tape->getHeadPosition();
    m_leftmostCell = headPosition - m_visibleCells / 2;
    
    updateTapeDisplay();
}

void TapeWidget::ensureCellVisible(int cellIndex)
{
    // Adjust leftmost cell if needed to make the specified cell visible
    if (cellIndex < m_leftmostCell) {
        m_leftmostCell = cellIndex;
    } else if (cellIndex >= m_leftmostCell + m_visibleCells) {
        m_leftmostCell = cellIndex - m_visibleCells + 1;
    }
    
    updateTapeDisplay();
}

void TapeWidget::updateCellSize()
{
    // Recalculate visible cells based on widget width and cell size
    m_visibleCells = width() / m_cellSize + 1;
    
    // Ensure head is visible
    if (m_tape) {
        ensureCellVisible(m_tape->getHeadPosition());
    }
}

void TapeWidget::drawCell(QPainter &painter, int cellIndex, const QRect &rect, char symbol)
{
    // Draw cell background
    if (cellIndex == m_tape->getHeadPosition()) {
        painter.fillRect(rect, QColor(255, 235, 185)); // Highlight current cell
    } else {
        painter.fillRect(rect, QColor(255, 255, 255));
    }
    
    // Draw cell border
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRect(rect);
    
    // Draw symbol
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(14);
    painter.setFont(font);
    
    // Convert symbol to QString for display
    QString symbolStr(symbol);
    
    // Center text in cell
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(symbolStr);
    painter.drawText(
        rect.left() + (rect.width() - textRect.width()) / 2,
        rect.top() + (rect.height() + fm.ascent() - fm.descent()) / 2,
        symbolStr
    );
    
    // Draw cell index below
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::gray);
    QString indexStr = QString::number(cellIndex);
    painter.drawText(
        rect.left() + (rect.width() - fm.horizontalAdvance(indexStr)) / 2,
        rect.bottom() - 5,
        indexStr
    );
}

void TapeWidget::drawHead(QPainter &painter)
{
    if (!m_tape) return;
    
    int headPosition = m_tape->getHeadPosition();
    QRect cellRect = getCellRect(headPosition);
    
    // Apply animation offset
    int offsetX = 0;
    if (m_headAnimOffset != 0) {
        offsetX = static_cast<int>(m_headAnimation * m_cellSize * m_headAnimOffset);
    }
    
    // Draw head triangle pointing down
    QPolygon triangle;
    int centerX = cellRect.left() + cellRect.width() / 2 + offsetX;
    triangle << QPoint(centerX, 0)
             << QPoint(centerX - 10, -15)
             << QPoint(centerX + 10, -15);
             
    painter.setBrush(QColor(255, 50, 50));
    painter.setPen(Qt::black);
    painter.drawPolygon(triangle);
    
    // Draw a small line connecting the triangle to the cell
    painter.drawLine(centerX, 0, centerX, 5);
}

void TapeWidget::drawGrid(QPainter &painter)
{
    // Draw vertical grid lines
    painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
    
    for (int i = 0; i <= m_visibleCells; ++i) {
        int x = i * m_cellSize;
        painter.drawLine(x, 0, x, height());
    }
    
    // Draw center line
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawLine(0, height() / 2, width(), height() / 2);
}