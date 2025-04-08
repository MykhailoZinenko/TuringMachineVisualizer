#ifndef TAPEWIDGET_H
#define TAPEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include "../model/Tape.h"

class TapeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal headAnimation READ headAnimation WRITE setHeadAnimation)

public:
    explicit TapeWidget(QWidget *parent = nullptr);
    ~TapeWidget();

    void setTape(Tape* tape);
    void updateTapeDisplay();
    void animateHeadMovement(bool moveRight);
    
    // For animation property
    qreal headAnimation() const { return m_headAnimation; }
    void setHeadAnimation(qreal value);

public slots:
    void onStepExecuted();
    void zoomIn();
    void zoomOut();
    void resetZoom();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Tape* m_tape;
    int m_visibleCells;       // Number of visible cells
    int m_cellSize;           // Size of each cell in pixels
    int m_leftmostCell;       // Index of leftmost visible cell
    int m_headAnimOffset;     // Animation offset for head movement
    qreal m_headAnimation;    // Animation property (0.0 to 1.0)
    
    QTimer* m_updateTimer;    // Timer for automatic updates
    QPropertyAnimation* m_headAnimationObj; // Animation object
    
    int xToCell(int x) const; // Convert x coordinate to cell index
    QRect getCellRect(int cellIndex) const; // Get rectangle for a cell
    void centerHeadPosition(); // Center the head in the view
    void ensureCellVisible(int cellIndex); // Make sure a cell is visible
    void ensureHeadVisible(); // Ensure the head is in view
    void updateCellSize(); // Recalculate cell size based on widget size
    
    // Drawing helpers
    void drawCell(QPainter &painter, int cellIndex, const QRect &rect, char symbol);
    void drawHead(QPainter &painter);
    void drawGrid(QPainter &painter);
};

#endif // TAPEWIDGET_H