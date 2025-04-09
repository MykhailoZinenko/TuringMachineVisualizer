#pragma once

#include <QWidget>

// Forward declarations
class QPainter;
class QPaintEvent;
class QTimer;
class QPropertyAnimation;
class QMouseEvent;
class QContextMenuEvent;
class QWheelEvent;
class QResizeEvent;
class Tape;

class TapeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal headAnimation READ headAnimation WRITE setHeadAnimation)

public:
    // Constructor & destructor
    explicit TapeWidget(QWidget *parent = nullptr);
    ~TapeWidget();

    // Core functionality
    void setTape(Tape* tape);
    void updateTapeDisplay();
    void animateHeadMovement(bool moveRight);

    // Animation property
    qreal headAnimation() const { return m_headAnimation; }
    void setHeadAnimation(qreal value);

    // Interactive mode
    void setInteractiveMode(bool enabled);
    bool isInteractiveMode() const { return m_interactiveMode; }

    // Zoom control
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void cellValueChanged(int position, char newValue);
    void headPositionChanged(int newPosition);
    void tapeModified();

public slots:
    void onStepExecuted();

protected:
    // Qt event handlers
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // Data
    Tape* m_tape;
    int m_visibleCells;
    int m_cellSize;
    int m_leftmostCell;
    int m_headAnimOffset;
    qreal m_headAnimation;
    bool m_interactiveMode;

    // UI components
    QTimer* m_updateTimer;
    QPropertyAnimation* m_headAnimationObj;

    // Helper methods
    int xToCell(int x) const;
    QRect getCellRect(int cellIndex) const;
    void centerHeadPosition();
    void ensureCellVisible(int cellIndex);
    void ensureHeadVisible();
    void updateCellSize();
    void editCellValue(int cellIndex);
    void moveHeadToCell(int cellIndex);

    // Drawing methods
    void drawCell(QPainter &painter, int cellIndex, const QRect &rect, char symbol);
    void drawHead(QPainter &painter);
    void drawGrid(QPainter &painter);
};