#pragma once

#include "keypad/layout.h"

#include <QFont>
#include <QWidget>
#include <array>
#include <functional>

class QVariantAnimation;

class ModeBar : public QWidget
{
public:
    ModeBar(Mode mode, std::function<void(Mode)> onChange, QWidget *parent = nullptr);

    /* Animates, never calls onChange. */
    void setMode(Mode mode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    Mode mode_;
    std::function<void(Mode)> onChange_;
    QVariantAnimation *slide_;
    qreal position_;
    QFont font_;
    std::array<QString, 2> labels_;
};
