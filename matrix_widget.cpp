#include "matrix_widget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QResizeEvent>

namespace {
double randDouble(double a, double b) {
    return a + (b - a) * QRandomGenerator::global()->generateDouble();
}
int randInt(int a, int b) {
    if (a >= b) return a;
    return a + QRandomGenerator::global()->bounded(b - a + 1);
}
}

MatrixWidget::MatrixWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(200, 150);

    // Sensible defaults (matrix preset)
    m_options.fadeStrength   = 0.05;
    m_options.rainDensity    = 0.5;
    m_options.windSpeed      = 0;

    m_options.raindrop.charArrays = {
        QStringLiteral(
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "アァカサタナハマヤャラワガザダバパ"
            "イィキシチニヒミリヰギジヂビピ"
            "ウゥクスツヌフムユュルグズブヅプ"
            "エェケセテネヘメレヱゲゼデベペ"
            "オォコソトノホモヨョロヲゴゾドボポヴッン")
    };
    m_options.raindrop.headColor  = QColor(255, 255, 255, 204);
    m_options.raindrop.trailColor = QColor(62, 225, 78, 255);
    m_options.raindrop.trailColors = { QColor(62, 225, 78, 255) };

    m_clock.start();

    m_timer.setInterval(qMax(8, m_options.minFrameTime / 3)); // ~ 60 fps
    connect(&m_timer, &QTimer::timeout, this, &MatrixWidget::tick);
    m_timer.start();
}

MatrixWidget::~MatrixWidget() {
    qDeleteAll(m_drops);
    m_drops.clear();
}

void MatrixWidget::setOptions(const MatrixOptions& opts) {
    m_options = opts;
    if (m_options.raindrop.trailColors.isEmpty() &&
        m_options.raindrop.trailColor.isValid())
        m_options.raindrop.trailColors = { m_options.raindrop.trailColor };

    applyOptionsLive();
    resetRaindrops();
}

void MatrixWidget::applyOptionsLive() {
    for (auto* d : m_drops)
        d->applyOptions(m_options.raindrop);
}

void MatrixWidget::setFontSizeAll(int size) {
    m_options.raindrop.fontSize        = size;
    m_options.raindrop.randomizeScale  = false;
    m_options.raindrop.minFontSize     = size;
    m_options.raindrop.maxFontSize     = size;

    for (auto* d : m_drops) {
        d->config().fontSize       = size;
        d->config().randomizeScale = false;
        d->config().minFontSize    = size;
        d->config().maxFontSize    = size;
        d->refreshFont();
    }
}

void MatrixWidget::setMoveSpeedAll(double speed) {
    m_options.raindrop.minMoveSpeed = speed;
    m_options.raindrop.maxMoveSpeed = speed;
    m_options.raindrop.moveSpeed    = speed;

    for (auto* d : m_drops) {
        d->config().minMoveSpeed = speed;
        d->config().maxMoveSpeed = speed;
        d->config().moveSpeed    = speed;
        d->setMoveSpeed(speed);
    }
}

void MatrixWidget::setHeadColorAll(const QColor& c) {
    m_options.raindrop.headColor = c;
    for (auto* d : m_drops) d->config().headColor = c;
}

void MatrixWidget::setTrailColorAll(const QColor& c) {
    m_options.raindrop.trailColor  = c;
    m_options.raindrop.trailColors = { c };
    for (auto* d : m_drops) {
        d->config().trailColor  = c;
        d->config().trailColors = { c };
        d->refreshTrailLength();
    }
}

void MatrixWidget::setFadeStrength(double v) {
    m_options.fadeStrength = qBound(0.0, v, 1.0);
}

void MatrixWidget::setDensity(double v) {
    m_options.rainDensity = qMax(0.01, v);
    resetRaindrops();
}

void MatrixWidget::setCharSet(const QStringList& sets) {
    if (sets.isEmpty()) return;
    m_options.raindrop.charArrays = sets;
    for (auto* d : m_drops) {
        d->config().charArrays = sets;
        d->randomizeChars();
    }
}

void MatrixWidget::resetRaindrops() {
    qDeleteAll(m_drops);
    m_drops.clear();
    createRaindrops();
}

void MatrixWidget::createRaindrops(int additional) {
    if (width() <= 0 || height() <= 0) return;

    const int cell = qMax(1, m_options.raindrop.rainWidth);
    const int cols = qMax(1, width() / cell);
    const bool vertical = (m_options.raindrop.direction == RainDirection::TopDown ||
                           m_options.raindrop.direction == RainDirection::BottomUp);

    if (additional < 0) {
        // Fresh creation — spread across columns
        additional = qMax(1, int(m_options.rainDensity * cols));
        if (additional > 1000) additional = 1000;
    }

    for (int i = 0; i < additional; ++i) {
        double x = 0, y = 0;

        if (vertical) {
            if (m_options.raindrop.alignToColumns)
                x = (i % cols) * cell;
            else
                x = randDouble(0, width());
            y = randDouble(0, height());
        } else {
            if (m_options.raindrop.alignToColumns) {
                const int rows = qMax(1, height() / cell);
                y = (i % rows) * cell;
            } else {
                y = randDouble(0, height());
            }
            x = randDouble(0, width());
        }

        m_drops.append(new MatrixRaindrop(x, y, m_options.raindrop, size()));
    }
}

void MatrixWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);

    const QSize s = e->size();
    if (s.width() <= 0 || s.height() <= 0) return;

    m_buffer = QImage(s, QImage::Format_ARGB32_Premultiplied);
    m_buffer.fill(Qt::black);

    // Rebuild rain for the new dimensions (cheap and reliable)
    resetRaindrops();
}

void MatrixWidget::setPaused(bool paused) {
    m_paused = paused;
    if (paused)
        m_timer.stop();
    else
        m_timer.start();
}

void MatrixWidget::tick() {
    if (m_paused || m_buffer.isNull()) return;

    const qint64 now = m_clock.elapsed();

    QPainter p(&m_buffer);
    p.setRenderHint(QPainter::Antialiasing,     false);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);

    // --- 1. Fade the buffer toward black ---
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    const int fadeAlpha = qBound(0, int(m_options.fadeStrength * 255.0), 255);
    p.fillRect(m_buffer.rect(), QColor(0, 0, 0, fadeAlpha));

    // --- 2. Micro alpha reduction to prevent residue ---
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(m_buffer.rect(), QColor(0, 0, 0, 5));
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    // --- 3. Optional wind scroll ---
    if (m_options.windSpeed > 0.0) {
        QImage snapshot = m_buffer;
        int dx = 0, dy = 0;
        switch (m_options.windDirection) {
        case RainDirection::LeftRight: dx =  int(m_options.windSpeed); break;
        case RainDirection::RightLeft: dx = -int(m_options.windSpeed); break;
        case RainDirection::TopDown:   dy =  int(m_options.windSpeed); break;
        case RainDirection::BottomUp:  dy = -int(m_options.windSpeed); break;
        }
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(m_buffer.rect(), Qt::black);
        p.drawImage(QPoint(dx, dy), snapshot);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    // --- 4. Redraw trails (so heads don't stay white) ---
    for (auto* d : m_drops)
        d->clear(p, m_options);

    // --- 5. Draw heads & advance ---
    for (auto* d : m_drops)
        d->step(p, m_options, now);

    p.end();
    update();
}

void MatrixWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    if (!m_buffer.isNull())
        p.drawImage(0, 0, m_buffer);
}