#pragma once

#include <QWidget>
#include <string>

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

/**
 * Widget for visualizing and interacting with a Turing machine tape
 */
class TapeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal headAnimation READ headAnimation WRITE setHeadAnimation)

public:
    /**
     * Constructor
     * @param parent The parent widget
     */
    explicit TapeWidget(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~TapeWidget();

    /**
     * Set the tape to visualize
     * @param tape Pointer to the tape (not owned by this widget)
     */
    void setTape(Tape* tape);

    /**
     * Update the tape display
     */
    void updateTapeDisplay();

    /**
     * Animate the head movement
     * @param moveRight True if moving right, false if moving left
     */
    void animateHeadMovement(bool moveRight);

    /**
     * Get the head animation property value
     * @return The animation value (0.0 to 1.0)
     */
    qreal headAnimation() const { return m_headAnimation; }

    /**
     * Set the head animation property value
     * @param value The animation value (0.0 to 1.0)
     */
    void setHeadAnimation(qreal value);

    /**
     * Set interactive mode (allows editing via clicks)
     * @param enabled True to enable interactive mode
     */
    void setInteractiveMode(bool enabled);

    /**
     * Check if interactive mode is enabled
     * @return True if interactive mode is enabled
     */
    bool isInteractiveMode() const { return m_interactiveMode; }

    /**
     * Zoom in
     */
    void zoomIn();

    /**
     * Zoom out
     */
    void zoomOut();

    /**
     * Reset zoom level
     */
    void resetZoom();

signals:
    /**
     * Signal emitted when a cell value is changed
     * @param position The cell position
     * @param newValue The new cell value
     */
    void cellValueChanged(int position, const std::string& newValue);

    /**
     * Signal emitted when the head position changes
     * @param newPosition The new head position
     */
    void headPositionChanged(int newPosition);

    /**
     * Signal emitted when the tape is modified
     */
    void tapeModified();

public slots:
    /**
     * Handle step execution
     */
    void onStepExecuted();

protected:
    /**
     * Paint event handler
     * @param event The paint event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * Mouse press event handler
     * @param event The mouse press event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * Mouse double click event handler
     * @param event The mouse double click event
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /**
     * Context menu event handler
     * @param event The context menu event
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * Mouse move event handler
     * @param event The mouse move event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * Wheel event handler
     * @param event The wheel event
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * Resize event handler
     * @param event The resize event
     */
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
    void drawCell(QPainter &painter, int cellIndex, const QRect &rect, const std::string& symbols);
    void drawHead(QPainter &painter);
    void drawGrid(QPainter &painter);
};