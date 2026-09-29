#pragma once

#include "entry/entry.h"

#include <QColor>
#include <QFont>
#include <QString>
#include <QWidget>

class QVariantAnimation;

class Readout : public QWidget
{
public:
    explicit Readout(QWidget *parent = nullptr);

    void present(const Entry &entry);
    void shake();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void showEditing(const Entry &entry);
    void showEquation(const Entry &entry);
    void fit();
    qreal shakeOffset() const;

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
