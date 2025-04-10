#include "TapeWidget.h"

// Qt includes
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QInputDialog>
#include <QMenu>
#include <QDebug>

// Project includes
#include "../model/Tape.h"

TapeWidget::TapeWidget(QWidget *parent)
    : QWidget(parent), m_tape(nullptr), m_visibleCells(15), m_cellSize(40),
      m_leftmostCell(0), m_headAnimOffset(0), m_headAnimation(0.0),
      m_interactiveMode(true)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &TapeWidget::updateTapeDisplay);
    m_updateTimer->start(100);

    m_headAnimationObj = new QPropertyAnimation(this, "headAnimation");
    m_headAnimationObj->setDuration(300);
    m_headAnimationObj->setEasingCurve(QEasingCurve::OutCubic);
}

TapeWidget::~TapeWidget()
{
    m_updateTimer->stop();
    delete m_headAnimationObj;
}

// Core functionality methods
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
        update();
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
    update();

    if (value >= 0.99) {
        m_headAnimation = 0.0;
        m_headAnimOffset = 0;
        ensureHeadVisible();
    }
}

// Interactive mode methods
void TapeWidget::setInteractiveMode(bool enabled)
{
    m_interactiveMode = enabled;
}

// Zoom control methods
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

// Qt event handlers
void TapeWidget::paintEvent(QPaintEvent *event)
{
    if (!m_tape) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(245, 245, 245));
    drawGrid(painter);

    int start = m_leftmostCell;
    int end = start + m_visibleCells;

    auto visibleCells = m_tape->getVisiblePortion(start, end - start);

    for (const auto& cellPair : visibleCells) {
        int cellIndex = cellPair.first;
        const std::string& symbols = cellPair.second;

        QRect cellRect = getCellRect(cellIndex);

        if (cellRect.intersects(event->rect())) {
            drawCell(painter, cellIndex, cellRect, symbols);
        }
    }

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
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    } else {
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

// Helper methods
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

// Drawing methods
void TapeWidget::drawCell(QPainter &painter, int cellIndex, const QRect &rect, const std::string& symbols)
{
    if (cellIndex == m_tape->getHeadPosition()) {
        painter.fillRect(rect, QColor(255, 235, 185));
    } else {
        painter.fillRect(rect, QColor(255, 255, 255));
    }

    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRect(rect);

    // Draw the symbols
    if (!symbols.empty() && symbols != m_tape->getBlankSymbolAsString()) {
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);

        QString symbolStr = QString::fromStdString(symbols);

        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(symbolStr);
        painter.drawText(
            rect.left() + (rect.width() - textRect.width()) / 2,
            rect.top() + (rect.height() + fm.ascent() - fm.descent()) / 2,
            symbolStr
        );
    }

    // Draw cell index at the bottom
    QFont smallFont = painter.font();
    smallFont.setPointSize(8);
    painter.setFont(smallFont);
    painter.setPen(Qt::gray);
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

    QPolygon triangle;
    int centerX = cellRect.left() + cellRect.width() / 2 + offsetX;
    triangle << QPoint(centerX, 0)
             << QPoint(centerX - 10, -15)
             << QPoint(centerX + 10, -15);

    painter.setBrush(QColor(255, 50, 50));
    painter.setPen(Qt::black);
    painter.drawPolygon(triangle);

    painter.drawLine(centerX, 0, centerX, 5);
}

void TapeWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));

    for (int i = 0; i <= m_visibleCells; ++i) {
        int x = i * m_cellSize;
        painter.drawLine(x, 0, x, height());
    }

    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawLine(0, height() / 2, width(), height() / 2);
}