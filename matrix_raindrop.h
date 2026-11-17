#pragma once
#include "matrix_types.h"
#include <QSize>
#include <QVector>
#include <QChar>
#include <QFont>

class QPainter;

/**
 * A single falling glyph with a short persistent trail.
 *
 * Lifecycle:
 *  - Constructed at a random position (or aligned to a column)
 *  - `clear()` is called every animation tick to redraw trail chars with
 *    the trail colour (so the head colour doesn't get stuck on the buffer)
 *  - `step()` is called every tick; it draws the head and advances the
 *    drop only when the per-drop `frameDelay` has elapsed.
 */
class MatrixRaindrop {
public:
    struct TrailChar {
        double x;
        double y;
        QChar  ch;
    };

    MatrixRaindrop(double x, double y,
                   const MatrixRaindropOptions& cfg,
                   const QSize& canvasSize);

    // Config / state
    void setCanvasSize(const QSize& s) { m_canvasSize = s; }
    const QSize& canvasSize() const { return m_canvasSize; }

    void applyOptions(const MatrixRaindropOptions& cfg);   // preserves scale if randomizeScale
    MatrixRaindropOptions& config() { return m_cfg; }
    const MatrixRaindropOptions& config() const { return m_cfg; }

    void setMoveSpeed(double s) { m_moveSpeed = s; }
    void refreshFont();
    void refreshTrailLength();

    void randomizeChars();
    void respawn(const MatrixOptions& global);

    // Rendering
    void clear(QPainter& painter, const MatrixOptions& global);
    void step (QPainter& painter, const MatrixOptions& global, qint64 nowMs);

    double  x() const { return m_x; }
    double  y() const { return m_y; }
    qint64  lastFrameTime() const { return m_lastFrameTime; }
    int     currentFrameDelay() const { return m_currentFrameDelay; }

private:
    double getMoveStep(const MatrixOptions& global) const;
    void   advance(const MatrixOptions& global);

    double  m_x;
    double  m_y;
    double  m_moveSpeed   = 4;
    QSize   m_canvasSize;
    MatrixRaindropOptions m_cfg;
    QString m_currentCharSet;
    QVector<TrailChar> m_trailChars;
    QFont   m_font;
    qint64  m_lastFrameTime = 0;
    int     m_currentFrameDelay = 50;
    int     m_trailLength = 1;
};