#pragma once
#include "matrix_types.h"
#include "matrix_raindrop.h"
#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QVector>
#include <QElapsedTimer>

/**
 * The animated canvas.  Uses an offscreen QImage buffer so the fade and
 * trail effects persist between paint events (like the HTML <canvas>).
 */
class MatrixWidget : public QWidget {
    Q_OBJECT
public:
    explicit MatrixWidget(QWidget* parent = nullptr);
    ~MatrixWidget() override;

    void setOptions(const MatrixOptions& opts);
    const MatrixOptions& options() const { return m_options; }
    MatrixOptions&       options()       { return m_options; }

    // Live-update helpers used by the control panel
    void setFontSizeAll(int size);
    void setMoveSpeedAll(double speed);           // applies min = max = speed
    void setHeadColorAll(const QColor& c);
    void setTrailColorAll(const QColor& c);
    void setFadeStrength(double v);
    void setDensity(double v);
    void setCharSet(const QStringList& sets);

    void resetRaindrops();
    void setPaused(bool paused);
    bool isPaused() const { return m_paused; }

    // Called by the control panel after touching `options()` directly
    void applyOptionsLive();

protected:
    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void tick();

private:
    void createRaindrops(int additional = -1);

    QImage                m_buffer;
    QTimer                m_timer;
    QVector<MatrixRaindrop*> m_drops;
    MatrixOptions         m_options;
    QElapsedTimer         m_clock;
    bool                  m_paused = false;
};