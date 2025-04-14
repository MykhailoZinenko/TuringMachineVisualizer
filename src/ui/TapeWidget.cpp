#include "TapeWidget.h"
#include "../model/Tape.h"
#include "../core/SessionManager.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QDebug>

TapeWidget::TapeWidget(QWidget *parent)
    : QWidget(parent), m_tape(nullptr), m_visibleCells(15), m_cellSize(40),
      m_leftmostCell(0), m_headAnimOffset(0), m_headAnimation(0.0),
      m_interactiveMode(true)
{
    // Set widget properties
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    // Create update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TapeWidget::updateTapeDisplay);
    m_updateTimer->start(100);

    // Create head animation object
    m_headAnimationObj = new QPropertyAnimation(this, "headAnimation");
    m_headAnimationObj->setDuration(300);
    m_headAnimationObj->setEasingCurve(QEasingCurve::OutCubic);
}

TapeWidget::~TapeWidget()
{
    // Stop the timer to prevent access to potentially deleted objects
    m_updateTimer->stop();

    // Delete animation object
    delete m_headAnimationObj;
}

void TapeWidget::setTape(Tape* tape)
{
    m_tape = tape;
    if (m_tape) {
        centerHeadPosition();
    }
    updateTapeDisplay();
}

void TapeWidget::updateTapeDisplay()
{
    update();
}

void TapeWidget::animateHeadMovement(bool moveRight)
{
    if (m_headAnimationObj->state() != QPropertyAnimation::Running) {
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
    update();

    if (value >= 0.99) {
        m_headAnimation = 0.0;
        m_headAnimOffset = 0;
        ensureHeadVisible();
    }
}

void TapeWidget::setInteractiveMode(bool enabled)
{
    m_interactiveMode = enabled;
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

void TapeWidget::onStepExecuted()
{
    if (!m_tape) return;
    ensureHeadVisible();
    updateTapeDisplay();
}

void TapeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), QColor(245, 245, 245));

    // Draw grid
    drawGrid(painter);

    // If no tape, just return
    if (!m_tape) return;

    // Calculate visible portion
    int start = m_leftmostCell;
    int end = start + m_visibleCells;

    auto visibleCells = m_tape->getVisiblePortion(start, end - start);

    // Draw cells
    for (const auto& cellPair : visibleCells) {
        int cellIndex = cellPair.first;
        const std::string& symbols = cellPair.second;

        QRect cellRect = getCellRect(cellIndex);
        drawCell(painter, cellIndex, cellRect, symbols);
    }

    // Draw head
    drawHead(painter);
}

void TapeWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int cellIndex = xToCell(event->pos().x());
        moveHeadToCell(cellIndex);
    }

    QWidget::mousePressEvent(event);
}

void TapeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    int cellIndex = xToCell(event->pos().x());
    editCellValue(cellIndex);

    QWidget::mouseDoubleClickEvent(event);
}

void TapeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_interactiveMode || !m_tape) {
        QWidget::contextMenuEvent(event);
        return;
    }

    int cellIndex = xToCell(event->pos().x());

    QMenu contextMenu(this);
    QAction* editAction = contextMenu.addAction(tr("Edit Cell Value"));
    QAction* moveHeadAction = contextMenu.addAction(tr("Move Head Here"));
    QAction* clearAction = contextMenu.addAction(tr("Clear Cell"));

    contextMenu.addSeparator();
    QAction* centerAction = contextMenu.addAction(tr("Center View on Head"));
    QAction* resetZoomAction = contextMenu.addAction(tr("Reset Zoom"));

    QAction* selectedAction = contextMenu.exec(event->globalPos());

    if (selectedAction == editAction) {
        editCellValue(cellIndex);
    } else if (selectedAction == moveHeadAction) {
        moveHeadToCell(cellIndex);
    } else if (selectedAction == clearAction) {
        if (m_tape) {
            int originalHeadPos = m_tape->getHeadPosition();
            m_tape->setHeadPosition(cellIndex);
            m_tape->write(m_tape->getBlankSymbolAsString());
            m_tape->setHeadPosition(originalHeadPos);
            updateTapeDisplay();
            emit cellValueChanged(cellIndex, m_tape->getBlankSymbolAsString());
            emit tapeModified();
        }
    } else if (selectedAction == centerAction) {
        centerHeadPosition();
        updateTapeDisplay();
    } else if (selectedAction == resetZoomAction) {
        resetZoom();
    }
}

void TapeWidget::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void TapeWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+wheel
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    } else {
        // Scroll with wheel
        if (event->angleDelta().y() > 0) {
            m_leftmostCell--;
        } else {
            m_leftmostCell++;
        }

        if (m_tape) {
            int headPosition = m_tape->getHeadPosition();
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

    int headPosition = m_tape->getHeadPosition();
    m_leftmostCell = headPosition - m_visibleCells / 2;

    updateTapeDisplay();
}

void TapeWidget::ensureCellVisible(int cellIndex)
{
    if (cellIndex < m_leftmostCell) {
        m_leftmostCell = cellIndex;
    } else if (cellIndex >= m_leftmostCell + m_visibleCells) {
        m_leftmostCell = cellIndex - m_visibleCells + 1;
    }

    updateTapeDisplay();
}

void TapeWidget::ensureHeadVisible()
{
    if (!m_tape) return;

    int headPosition = m_tape->getHeadPosition();

    if (headPosition < m_leftmostCell || headPosition >= m_leftmostCell + m_visibleCells) {
        centerHeadPosition();
    }
}

void TapeWidget::updateCellSize()
{
    m_visibleCells = width() / m_cellSize + 1;

    if (m_tape) {
        ensureCellVisible(m_tape->getHeadPosition());
    }
}

void TapeWidget::editCellValue(int cellIndex)
{
    if (!m_tape) return;

    int originalHeadPos = m_tape->getHeadPosition();
    m_tape->setHeadPosition(cellIndex);
    std::string currentSymbols = m_tape->read();

    QString currentValue = (currentSymbols == m_tape->getBlankSymbolAsString()) ? "" : QString::fromStdString(currentSymbols);

    bool ok;
    QString newValue = QInputDialog::getText(this, tr("Edit Cell Value"),
                                           tr("Enter new cell value (empty for blank):"),
                                           QLineEdit::Normal,
                                           currentValue, &ok);

    if (ok) {
        std::string newSymbols;
        if (newValue.isEmpty()) {
            newSymbols = m_tape->getBlankSymbolAsString();
        } else {
            newSymbols = newValue.toStdString();
        }

        m_tape->write(newSymbols);
        m_tape->setHeadPosition(originalHeadPos);
        updateTapeDisplay();
        emit cellValueChanged(cellIndex, newSymbols);
        emit tapeModified();
    } else {
        m_tape->setHeadPosition(originalHeadPos);
    }
}

void TapeWidget::moveHeadToCell(int cellIndex)
{
    if (!m_tape) return;

    int oldPosition = m_tape->getHeadPosition();

    if (cellIndex != oldPosition) {
        bool movingRight = cellIndex > oldPosition;
        m_tape->setHeadPosition(cellIndex);
        animateHeadMovement(movingRight);
        emit headPositionChanged(cellIndex);
        emit tapeModified();
    }
}

void TapeWidget::drawCell(QPainter &painter, int cellIndex, const QRect &rect, const std::string& symbols)
{
    if (!m_tape) return;

    bool isCurrentCell = (cellIndex == m_tape->getHeadPosition());

    // Cell background and shadow
    QColor backgroundColor;

    if (isCurrentCell) {
        // Gradient for current cell
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, QColor(219, 234, 254));  // Light blue
        gradient.setColorAt(1, QColor(191, 219, 254));  // Slightly darker blue
        painter.setBrush(gradient);
    } else {
        backgroundColor = QColor(255, 255, 255);  // White for normal cells
        painter.setBrush(backgroundColor);
    }

    // Draw cell with rounded corners
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 4, 4);

    // Draw border with subtle shadow effect
    QPen borderPen;
    if (isCurrentCell) {
        borderPen = QPen(QColor(59, 130, 246), 2);  // Blue border for current cell
    } else {
        borderPen = QPen(QColor(203, 213, 225), 1);  // Light gray for other cells
    }
    painter.setPen(borderPen);
    painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 4, 4);

    // Draw the symbols with nice typography
    if (!symbols.empty() && symbols != m_tape->getBlankSymbolAsString()) {
        QString symbolStr = QString::fromStdString(symbols);

        // Set font
        QFont font = painter.font();
        font.setFamily("Inter");
        font.setPointSize(14);
        if (isCurrentCell) {
            font.setBold(true);
        }
        painter.setFont(font);

        // Set text color
        painter.setPen(isCurrentCell ? QColor(30, 64, 175) : QColor(15, 23, 42));

        // Draw centered text
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(symbolStr);
        painter.drawText(
            rect.left() + (rect.width() - textRect.width()) / 2,
            rect.top() + (rect.height() + fm.ascent() - fm.descent()) / 2,
            symbolStr
        );
    }

    // Draw cell index at the bottom with subtle styling
    QFont smallFont = painter.font();
    smallFont.setFamily("Inter");
    smallFont.setPointSize(8);
    painter.setFont(smallFont);
    painter.setPen(QColor(148, 163, 184));  // Subtle gray
    QString indexStr = QString::number(cellIndex);
    QFontMetrics smallFm(smallFont);
    painter.drawText(
        rect.left() + (rect.width() - smallFm.horizontalAdvance(indexStr)) / 2,
        rect.bottom() - 5,
        indexStr
    );
}

void TapeWidget::drawHead(QPainter &painter)
{
    if (!m_tape) return;

    int headPosition = m_tape->getHeadPosition();
    QRect cellRect = getCellRect(headPosition);

    int offsetX = 0;
    if (m_headAnimOffset != 0) {
        offsetX = static_cast<int>(m_headAnimation * m_cellSize * m_headAnimOffset);
    }

    // Draw a more refined head using a custom path
    int centerX = cellRect.left() + cellRect.width() / 2 + offsetX;
    int topY = 0;

    // Create a custom path for the head
    QPainterPath path;

    // Triangle part
    path.moveTo(centerX, topY);
    path.lineTo(centerX - 10, topY - 14);
    path.lineTo(centerX + 10, topY - 14);
    path.closeSubpath();

    // Create a gradient
    QLinearGradient gradient(QPointF(centerX, topY - 14), QPointF(centerX, topY));
    gradient.setColorAt(0, QColor(239, 68, 68));  // Lighter red at top
    gradient.setColorAt(1, QColor(185, 28, 28));  // Darker red at bottom

    // Draw the head with shadow
    painter.setPen(Qt::NoPen);

    // Draw shadow first
    painter.save();
    painter.translate(2, 2);
    painter.setBrush(QColor(0, 0, 0, 50));  // Semi-transparent black
    painter.drawPath(path);
    painter.restore();

    // Draw the actual head
    painter.setBrush(gradient);
    painter.drawPath(path);

    // Add a subtle outline
    painter.setPen(QPen(QColor(153, 27, 27), 1));
    painter.drawPath(path);

    // Draw a stem with rounded cap
    QPen stemPen(QColor(153, 27, 27), 2, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(stemPen);
    painter.drawLine(centerX, topY, centerX, topY + 5);
}

void TapeWidget::drawGrid(QPainter &painter)
{
    // Draw dotted grid lines
    painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));

    for (int i = 0; i <= m_visibleCells; ++i) {
        int x = i * m_cellSize;
        painter.drawLine(x, 0, x, height());
    }

    // Draw center horizontal line
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawLine(0, height() / 2, width(), height() / 2);
}