#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QColorDialog>
#include <QGroupBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_matrix = new MatrixWidget(this);

    MatrixOptions opts;

    // ── Defaults: the classic "matrix" preset ───────────────────────
    opts.raindrop.charArrays = {
        QStringLiteral(
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "アァカサタナハマヤャラワガザダバパ"
            "イィキシチニヒミリヰギジヂビピ"
            "ウゥクスツヌフムユュルグズブヅプ"
            "エェケセテネヘメレヱゲゼデベペ"
            "オォコソトノホモヨョロヲゴゾドボポヴッン")
    };
    opts.raindrop.headColor        = m_headColor;
    opts.raindrop.trailColor       = m_trailColor;
    opts.raindrop.trailColors      = { m_trailColor };
    opts.raindrop.fontSize         = 16;
    opts.raindrop.fontFamily       = QStringLiteral("Monospace");
    opts.raindrop.moveSpeed        = 0;
    opts.raindrop.minMoveSpeed     = 8;
    opts.raindrop.maxMoveSpeed     = 8;
    opts.raindrop.frameDelay       = 50;
    opts.raindrop.minFrameDelay    = 30;
    opts.raindrop.maxFrameDelay    = 60;
    opts.raindrop.randomizeFrameDelay = true;
    opts.raindrop.randomizePosition   = true;
    opts.raindrop.alignToColumns      = true;
    opts.fadeStrength              = 0.05;
    opts.rainDensity               = 0.5;

    m_matrix->setOptions(opts);

    // ── Layout ──────────────────────────────────────────────────────
    QWidget* central = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_matrix, 1);
    mainLayout->addWidget(createControlPanel());

    setCentralWidget(central);
    setWindowTitle(QStringLiteral("Matrix Animation — Qt"));
    resize(1280, 800);
}

QWidget* MainWindow::createControlPanel() {
    QWidget* panel = new QWidget;
    panel->setFixedWidth(300);
    panel->setStyleSheet(R"(
        QWidget {
            background-color: #0f0f0f;
            color: #d8ffd8;
            font-family: "Consolas", "Menlo", "Monospace";
        }
        QGroupBox {
            border: 1px solid #2a4a2a;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 8px;
            font-weight: bold;
            color: #82ffa9;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QSlider::groove:horizontal {
            height: 4px; background: #1e3a1e; border-radius: 2px;
        }
        QSlider::handle:horizontal {
            width: 14px; margin: -6px 0; border-radius: 7px;
            background: #46d446;
        }
        QPushButton {
            background-color: #163316;
            border: 1px solid #2e5e2e;
            border-radius: 4px;
            padding: 6px;
        }
        QPushButton:hover { background-color: #1f4d1f; }
        QComboBox {
            background-color: #163316;
            border: 1px solid #2e5e2e;
            border-radius: 4px;
            padding: 4px;
        }
    )");

    QVBoxLayout* v = new QVBoxLayout(panel);
    v->setContentsMargins(14, 14, 14, 14);
    v->setSpacing(10);

    QLabel* title = new QLabel("MATRIX CONTROL");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #82ffa9;");
    v->addWidget(title);

    // ── Font size ───────────────────────────────────────────────────
    v->addWidget(new QLabel("Font Size (px)"));
    m_fontSlider = new QSlider(Qt::Horizontal);
    m_fontSlider->setRange(6, 60);
    m_fontSlider->setValue(16);
    v->addWidget(m_fontSlider);
    connect(m_fontSlider, &QSlider::valueChanged,
            this, &MainWindow::onFontSizeChanged);

    // ── Speed ───────────────────────────────────────────────────────
    v->addWidget(new QLabel("Speed (px / step)"));
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(1, 40);
    m_speedSlider->setValue(8);
    v->addWidget(m_speedSlider);
    connect(m_speedSlider, &QSlider::valueChanged,
            this, &MainWindow::onSpeedChanged);

    // ── Density ─────────────────────────────────────────────────────
    v->addWidget(new QLabel("Density (0.1 – 5.0)"));
    m_densitySlider = new QSlider(Qt::Horizontal);
    m_densitySlider->setRange(1, 50);   // 1..50 → 0.1..5.0
    m_densitySlider->setValue(5);       // 0.5
    v->addWidget(m_densitySlider);
    connect(m_densitySlider, &QSlider::valueChanged,
            this, &MainWindow::onDensityChanged);

    // ── Fade strength ───────────────────────────────────────────────
    v->addWidget(new QLabel("Fade Strength (0.01 – 0.50)"));
    m_fadeSlider = new QSlider(Qt::Horizontal);
    m_fadeSlider->setRange(1, 50);
    m_fadeSlider->setValue(5);
    v->addWidget(m_fadeSlider);
    connect(m_fadeSlider, &QSlider::valueChanged,
            this, &MainWindow::onFadeChanged);

    // ── Character set ───────────────────────────────────────────────
    v->addWidget(new QLabel("Character Set"));
    m_charCombo = new QComboBox;
    m_charCombo->addItem("Matrix (Hex + Katakana)");
    m_charCombo->addItem("Binary (01)");
    m_charCombo->addItem("Hex (0-F)");
    m_charCombo->addItem("Digits only");
    m_charCombo->addItem("Braille");
    m_charCombo->addItem("Symbols");
    m_charCombo->addItem("DNA (ACGT)");
    v->addWidget(m_charCombo);
    connect(m_charCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCharSetChanged);

    // ── Colors ──────────────────────────────────────────────────────
    v->addWidget(new QLabel("Head Color"));
    m_headColorBtn = new QPushButton;
    m_headColorBtn->setFixedHeight(34);
    v->addWidget(m_headColorBtn);
    connect(m_headColorBtn, &QPushButton::clicked,
            this, &MainWindow::onHeadColorClicked);

    v->addWidget(new QLabel("Trail Color"));
    m_trailColorBtn = new QPushButton;
    m_trailColorBtn->setFixedHeight(34);
    v->addWidget(m_trailColorBtn);
    connect(m_trailColorBtn, &QPushButton::clicked,
            this, &MainWindow::onTrailColorClicked);

    v->addStretch(1);

    // ── Buttons ─────────────────────────────────────────────────────
    m_pauseBtn = new QPushButton("Pause");
    v->addWidget(m_pauseBtn);
    connect(m_pauseBtn, &QPushButton::clicked,
            this, &MainWindow::onPauseClicked);

    m_resetBtn = new QPushButton("Reset Raindrops");
    v->addWidget(m_resetBtn);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &MainWindow::onResetClicked);

    // Initialize the color button visuals
    updateColorButton(m_headColorBtn,  m_headColor);
    updateColorButton(m_trailColorBtn, m_trailColor);

    return panel;
}

void MainWindow::updateColorButton(QPushButton* btn, const QColor& c) {
    btn->setText(c.name(QColor::HexRgb).toUpper());
    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; "
        "border: 1px solid #555; border-radius: 4px; }")
        .arg(c.name(QColor::HexRgb),
             (c.lightness() > 128) ? "#000000" : "#ffffff"));
}

void MainWindow::onFontSizeChanged(int v) {
    m_matrix->setFontSizeAll(v);
}

void MainWindow::onSpeedChanged(int v) {
    m_matrix->setMoveSpeedAll(double(v));
}

void MainWindow::onDensityChanged(int v) {
    m_matrix->setDensity(v / 10.0);
}

void MainWindow::onFadeChanged(int v) {
    m_matrix->setFadeStrength(v / 100.0);
}

void MainWindow::onHeadColorClicked() {
    QColor c = QColorDialog::getColor(m_headColor, this,
                                      "Head Color",
                                      QColorDialog::ShowAlphaChannel);
    if (!c.isValid()) return;
    m_headColor = c;
    m_matrix->setHeadColorAll(c);
    updateColorButton(m_headColorBtn, c);
}

void MainWindow::onTrailColorClicked() {
    QColor c = QColorDialog::getColor(m_trailColor, this,
                                      "Trail Color",
                                      QColorDialog::ShowAlphaChannel);
    if (!c.isValid()) return;
    m_trailColor = c;
    m_matrix->setTrailColorAll(c);
    updateColorButton(m_trailColorBtn, c);
}

void MainWindow::onCharSetChanged(int idx) {
    QStringList sets;
    switch (idx) {
    case 0: // matrix preset
        sets = { QStringLiteral(
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "アァカサタナハマヤャラワガザダバパ"
            "イィキシチニヒミリヰギジヂビピ"
            "ウゥクスツヌフムユュルグズブヅプ"
            "エェケセテネヘメレヱゲゼデベペ"
            "オォコソトノホモヨョロヲゴゾドボポヴッン") };
        break;
    case 1: sets = { QStringLiteral("01") }; break;
    case 2: sets = { QStringLiteral("0123456789ABCDEF") }; break;
    case 3: sets = { QStringLiteral("0123456789") }; break;
    case 4: sets = { QStringLiteral(
        "⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏"
        "⠐⠑⠒⠓⠔⠕⠖⠗⠘⠙⠚⠛⠜⠝⠞⠟"
        "⠠⠡⠢⠣⠤⠥⠦⠧⠨⠩⠪⠫⠬⠭⠮⠯") };
        break;
    case 5: sets = { QStringLiteral("+=#%*/\\[]{}()&^$@!~`'\"|<>,.?") }; break;
    case 6: sets = { QStringLiteral("ACGT") }; break;
    }
    m_matrix->setCharSet(sets);
}

void MainWindow::onResetClicked() {
    m_matrix->resetRaindrops();
}

void MainWindow::onPauseClicked() {
    const bool pause = !m_matrix->isPaused();
    m_matrix->setPaused(pause);
    m_pauseBtn->setText(pause ? "Resume" : "Pause");
}