#include "matrix_raindrop.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetricsF>

namespace {
double randDouble(double a, double b) {
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}
int randInt(int a, int b) {
    if (a >= b) return a;
    return a + QRandomGenerator::global()->bounded(b - a + 1);
}
}

MatrixRaindrop::MatrixRaindrop(double x, double y,
                               const MatrixRaindropOptions& cfg,
                               const QSize& canvasSize)
    : m_x(x), m_y(y), m_canvasSize(canvasSize), m_cfg(cfg)
{
    // Ensure trail colour list is populated
    if (m_cfg.trailColors.isEmpty() && m_cfg.trailColor.isValid())
        m_cfg.trailColors = { m_cfg.trailColor };
    if (m_cfg.trailColors.isEmpty())
        m_cfg.trailColors = { QColor(140, 62, 225) };

    // Trail length = number of trail colours (one glyph per colour)
    m_trailLength = qMax(1, m_cfg.trailColors.size());

    // Randomize initial scale if requested
    if (m_cfg.randomizeScale && m_cfg.maxFontSize > m_cfg.minFontSize)
        m_cfg.fontSize = randInt(m_cfg.minFontSize, m_cfg.maxFontSize);

    // Initial frame delay
    m_currentFrameDelay = m_cfg.randomizeFrameDelay
                              ? randInt(m_cfg.minFrameDelay, m_cfg.maxFrameDelay)
                              : m_cfg.frameDelay;

    // Initial speed
    if (m_cfg.maxMoveSpeed > m_cfg.minMoveSpeed)
        m_moveSpeed = randDouble(m_cfg.minMoveSpeed, m_cfg.maxMoveSpeed);
    else if (m_cfg.moveSpeed > 0)
        m_moveSpeed = m_cfg.moveSpeed;
    else
        m_moveSpeed = 4;

    refreshFont();
    randomizeChars();
}

void MatrixRaindrop::refreshFont() {
    m_font = QFont(m_cfg.fontFamily);
    m_font.setPixelSize(m_cfg.fontSize);
    m_font.setStyleStrategy(QFont::NoAntialias);
}

void MatrixRaindrop::refreshTrailLength() {
    if (m_cfg.trailColors.isEmpty() && m_cfg.trailColor.isValid())
        m_cfg.trailColors = { m_cfg.trailColor };
    if (m_cfg.trailColors.isEmpty())
        m_cfg.trailColors = { QColor(140, 62, 225) };
    m_trailLength = qMax(1, m_cfg.trailColors.size());
}

void MatrixRaindrop::applyOptions(const MatrixRaindropOptions& cfg) {
    const int preservedFontSize = m_cfg.fontSize;

    m_cfg = cfg;

    // Preserve individual randomized font size if randomizeScale is still on
    if (m_cfg.randomizeScale &&
        preservedFontSize >= m_cfg.minFontSize &&
        preservedFontSize <= m_cfg.maxFontSize)
    {
        m_cfg.fontSize = preservedFontSize;
    }

    refreshFont();
    refreshTrailLength();
}

void MatrixRaindrop::randomizeChars() {
    if (m_cfg.charArrays.isEmpty()) {
        m_currentCharSet = QStringLiteral("0123456789");
    } else {
        int idx = randInt(0, m_cfg.charArrays.size() - 1);
        m_currentCharSet = m_cfg.charArrays[idx];
    }
}

void MatrixRaindrop::respawn(const MatrixOptions& global) {
    (void)global;

    if (m_cfg.charArrays.size() > 1)
        randomizeChars();

    if (m_cfg.randomizeFrameDelay)
        m_currentFrameDelay = randInt(m_cfg.minFrameDelay, m_cfg.maxFrameDelay);

    if (m_cfg.randomizeScale && m_cfg.maxFontSize > m_cfg.minFontSize) {
        m_cfg.fontSize = randInt(m_cfg.minFontSize, m_cfg.maxFontSize);
        refreshFont();
    }

    // Speed
    if (m_cfg.maxMoveSpeed > m_cfg.minMoveSpeed) {
        if (m_cfg.correlateScaleSpeed != SpeedScaleCorrelation::Off &&
            m_cfg.randomizeScale && m_cfg.maxFontSize > m_cfg.minFontSize)
        {
            const double t = double(m_cfg.fontSize - m_cfg.minFontSize) /
                             double(m_cfg.maxFontSize - m_cfg.minFontSize);
            const double range = m_cfg.maxMoveSpeed - m_cfg.minMoveSpeed;
            m_moveSpeed = (m_cfg.correlateScaleSpeed == SpeedScaleCorrelation::Inverse)
                              ? (m_cfg.maxMoveSpeed - t * range)
                              : (m_cfg.minMoveSpeed + t * range);
        } else {
            m_moveSpeed = randDouble(m_cfg.minMoveSpeed, m_cfg.maxMoveSpeed);
        }
    }

    // Randomize starting position on the perpendicular axis
    if (m_cfg.randomizePosition) {
        const bool vertical = (m_cfg.direction == RainDirection::TopDown ||
                               m_cfg.direction == RainDirection::BottomUp);
        const int cell = qMax(1, m_cfg.rainWidth);

        if (vertical) {
            const int cols = qMax(1, m_canvasSize.width() / cell);
            m_x = m_cfg.alignToColumns
                      ? randInt(0, cols - 1) * cell
                      : randDouble(0, m_canvasSize.width());
        } else {
            const int rows = qMax(1, m_canvasSize.height() / cell);
            m_y = m_cfg.alignToColumns
                      ? randInt(0, rows - 1) * cell
                      : randDouble(0, m_canvasSize.height());
        }
    }
}

double MatrixRaindrop::getMoveStep(const MatrixOptions& global) const {
    if (m_cfg.maxMoveSpeed > m_cfg.minMoveSpeed || m_cfg.moveSpeed > 0)
        return m_moveSpeed;

    // Fallback: use cell size in the direction of travel
    if (m_cfg.direction == RainDirection::TopDown ||
        m_cfg.direction == RainDirection::BottomUp)
        return global.rainHeight > 0 ? global.rainHeight : 10;

    return global.rainWidth > 0 ? global.rainWidth : 10;
}

void MatrixRaindrop::clear(QPainter& painter, const MatrixOptions& global) {
    painter.setFont(m_font);

    for (int i = 0; i < m_trailChars.size(); ++i) {
        const TrailChar& tc = m_trailChars[i];

        QColor c;
        if (global.trailColorLogic == TrailColorLogic::Sequential) {
            c = m_cfg.trailColors[qMin(i, m_cfg.trailColors.size() - 1)];
        } else {
            c = m_cfg.trailColors[randInt(0, m_cfg.trailColors.size() - 1)];
        }

        painter.setPen(c);
        const QFontMetricsF fm(m_font);
        const QString s(tc.ch);
        const double w = fm.horizontalAdvance(s);
        painter.drawText(QPointF(tc.x - w / 2.0, tc.y), s);
    }
}

void MatrixRaindrop::step(QPainter& painter,
                          const MatrixOptions& global,
                          qint64 nowMs)
{
    if (nowMs - m_lastFrameTime < m_currentFrameDelay)
        return;

    // Pick a random glyph
    const QChar ch = m_currentCharSet.isEmpty()
                         ? QChar('0')
                         : m_currentCharSet[randInt(0, m_currentCharSet.size() - 1)];

    // Add to trail, trim
    m_trailChars.prepend({ m_x, m_y, ch });
    while (m_trailChars.size() > m_trailLength)
        m_trailChars.removeLast();

    // Draw head
    painter.setFont(m_font);
    painter.setPen(m_cfg.headColor);

    const QFontMetricsF fm(m_font);
    const QString s(ch);
    const double w = fm.horizontalAdvance(s);
    painter.drawText(QPointF(m_x - w / 2.0, m_y), s);

    // Optional jitter
    if (m_cfg.jitterLeftStrength || m_cfg.jitterRightStrength)
        m_x += randDouble(-m_cfg.jitterLeftStrength, m_cfg.jitterRightStrength);
    if (m_cfg.jitterUpStrength || m_cfg.jitterDownStrength)
        m_y += randDouble(-m_cfg.jitterUpStrength, m_cfg.jitterDownStrength);

    advance(global);
    m_lastFrameTime = nowMs;
}

void MatrixRaindrop::advance(const MatrixOptions& global) {
    const double stepPx = getMoveStep(global);

    switch (m_cfg.direction) {
    case RainDirection::TopDown:
        m_y += stepPx;
        if (m_y > m_canvasSize.height()) {
            m_y = randDouble(-100, 0);
            respawn(global);
        }
        break;

    case RainDirection::BottomUp:
        m_y -= stepPx;
        if (m_y < 0) {
            m_y = randDouble(m_canvasSize.height(),
                             m_canvasSize.height() + 100);
            respawn(global);
        }
        break;

    case RainDirection::LeftRight:
        m_x += stepPx;
        if (m_x > m_canvasSize.width()) {
            m_x = randDouble(-100, 0);
            respawn(global);
        }
        break;

    case RainDirection::RightLeft:
        m_x -= stepPx;
        if (m_x < 0) {
            m_x = randDouble(m_canvasSize.width(),
                             m_canvasSize.width() + 100);
            respawn(global);
        }
        break;
    }
}