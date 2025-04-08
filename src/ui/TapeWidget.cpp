#include "TapeWidget.h"

#include <iostream>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDebug>

TapeWidget::TapeWidget(QWidget *parent)
    : QWidget(parent), m_tape(nullptr), m_visibleCells(15), m_cellSize(40),
      m_leftmostCell(0), m_headAnimOffset(0), m_headAnimation(0.0)
{
    // Setup widget
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    
    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TapeWidget::updateTapeDisplay);
    m_updateTimer->start(100); // Update every 100ms
    
    // Setup animation
    m_headAnimationObj = new QPropertyAnimation(this, "headAnimation");
    m_headAnimationObj->setDuration(300); // 300ms animation
    m_headAnimationObj->setEasingCurve(QEasingCurve::OutCubic);
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
        update(); // Request a repaint
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

void TapeWidget::setHeadAnimation(qreal value)
{
    m_headAnimation = value;
    update(); // Request repaint when animation value changes
    
    // When animation completes, ensure cell is visible
    if (value >= 0.99) {
        m_headAnimation = 0.0;
        m_headAnimOffset = 0;
        ensureCellVisible(m_tape->getHeadPosition());
    }
}

void TapeWidget::onStepExecuted()
{
    // This will be called after a step is executed
    centerHeadPosition();
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

void TapeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Convert click position to cell index
        int cellIndex = xToCell(event->pos().x());
        
        // Move head to clicked cell (disabled for now)
        // if (m_tape) {
        //     m_tape->setHeadPosition(cellIndex);
        //     updateTapeDisplay();
        // }
    }
    
    QWidget::mousePressEvent(event);
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