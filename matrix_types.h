#pragma once
#include <QColor>
#include <QString>
#include <QStringList>
#include <QList>

enum class RainDirection {
    TopDown,    // TD
    BottomUp,   // BU
    LeftRight,  // LR
    RightLeft   // RL
};

enum class TrailColorLogic { Random, Sequential };
enum class SpeedScaleCorrelation { Off, Proportional, Inverse };

/**
 * Per-raindrop configuration. Mirrors `MatrixRaindropOptions` in the TS source.
 */
struct MatrixRaindropOptions {
    RainDirection direction        = RainDirection::TopDown;

    // Character sets — one is chosen per-drop on respawn
    QStringList charArrays;

    QColor headColor               = QColor(255, 255, 255, 204);
    QColor trailColor              = QColor(140, 62, 225, 255);
    QList<QColor> trailColors;     // overrides trailColor if non-empty

    QString fontFamily             = QStringLiteral("Monospace");
    int  fontSize                  = 16;
    int  minFontSize               = 12;
    int  maxFontSize               = 22;
    bool randomizeScale            = false;

    double moveSpeed               = 0;
    double minMoveSpeed            = 4;
    double maxMoveSpeed            = 12;
    SpeedScaleCorrelation correlateScaleSpeed = SpeedScaleCorrelation::Off;

    int  frameDelay                = 50;
    int  minFrameDelay             = 30;
    int  maxFrameDelay             = 60;
    bool randomizeFrameDelay       = false;
    bool randomizePosition         = false;
    bool alignToColumns            = true;

    int  rainWidth                 = 12;

    int  jitterLeftStrength        = 0;
    int  jitterRightStrength       = 0;
    int  jitterUpStrength          = 0;
    int  jitterDownStrength        = 0;
};

/**
 * Global animation settings. Mirrors `MatrixOptions`.
 */
struct MatrixOptions {
    double fadeStrength            = 0.05;
    double rainDensity             = 0.5;
    int    rainWidth               = 10;
    int    rainHeight              = 18;

    int    minFrameTime            = 50;   // ms between timer ticks (via QTimer)

    double windSpeed               = 0;
    RainDirection windDirection    = RainDirection::TopDown;

    TrailColorLogic trailColorLogic = TrailColorLogic::Random;

    bool   autoStart               = true;

    int    headBloomSize           = 0;
    QColor headBloomColor          = QColor(255, 255, 255);
    int    trailBloomSize          = 0;
    QColor trailBloomColor         = QColor(255, 255, 255);

    MatrixRaindropOptions raindrop;
};