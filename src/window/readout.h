#pragma once

#include "entry/entry.h"

#include <QColor>
#include <QFont>
#include <QString>
#include <QWidget>
#include <functional>

class QVariantAnimation;

class Readout : public QWidget
{
public:
    Readout(std::function<void()> onCopy, std::function<void()> onClear, QWidget *parent = nullptr);

    void present(const Entry &entry);
    void shake();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void showEditing(const Entry &entry);
    void showEquation(const Entry &entry);
    void fit();
    qreal shakeOffset() const;
    void copy();
    void openMenu(QPoint position);

    std::function<void()> onCopy_;
    std::function<void()> onClear_;
    Phase phase_ = Phase::Editing;
    QString top_;
    QString topShown_;
    QColor topColor_;
    QString main_;
    QString mainShown_;
    QString ghost_;
    QColor mainColor_;
    int mainPixels_ = 0;
    qreal ghostWidth_ = 0;
    QFont topFont_;
    QFont mainFont_;
    QVariantAnimation *rise_;
    QVariantAnimation *shake_;
};
