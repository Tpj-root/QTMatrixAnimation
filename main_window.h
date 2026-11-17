#pragma once
#include <QMainWindow>
#include "matrix_widget.h"

class QSlider;
class QPushButton;
class QComboBox;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onFontSizeChanged(int v);
    void onSpeedChanged(int v);
    void onDensityChanged(int v);
    void onFadeChanged(int v);
    void onHeadColorClicked();
    void onTrailColorClicked();
    void onCharSetChanged(int idx);
    void onResetClicked();
    void onPauseClicked();

private:
    QWidget* createControlPanel();
    void     updateColorButton(QPushButton* btn, const QColor& c);

    MatrixWidget* m_matrix   = nullptr;

    QSlider*      m_fontSlider    = nullptr;
    QSlider*      m_speedSlider   = nullptr;
    QSlider*      m_densitySlider = nullptr;
    QSlider*      m_fadeSlider    = nullptr;
    QComboBox*    m_charCombo     = nullptr;
    QPushButton*  m_headColorBtn  = nullptr;
    QPushButton*  m_trailColorBtn = nullptr;
    QPushButton*  m_pauseBtn      = nullptr;
    QPushButton*  m_resetBtn      = nullptr;

    QColor        m_headColor  = QColor(255, 255, 255, 204);
    QColor        m_trailColor = QColor(62, 225, 78, 255);
};