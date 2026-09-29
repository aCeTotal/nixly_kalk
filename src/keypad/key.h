#pragma once

#include "keypad/layout.h"

#include <QAbstractButton>
#include <QFont>
#include <QString>

class QPainter;
class QVariantAnimation;

class Key : public QAbstractButton
{
public:
    explicit Key(const KeySpec &spec, QWidget *parent = nullptr);

    const KeySpec &spec() const { return spec_; }
    /* Alternate stroke while shifted. */
    Keystroke stroke() const;
    void toggleAlternate();
    void setLabel(const QString &label);
    void flash();
    void setSelected(bool selected);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void resizeEvent(QResizeEvent *event) override;
    void paintFace(QPainter &painter, const QRectF &face, qreal radius) const;
    void paintRipple(QPainter &painter, const QRectF &face, qreal radius) const;
    void paintSelection(QPainter &painter, const QRectF &face, qreal radius) const;
    void startRipple(QPointF origin);

    KeySpec spec_;
    QString label_;
    QString altLabel_;
    bool alternate_ = false;
    bool selected_ = false;
    QFont font_;
    QVariantAnimation *hover_;
    QVariantAnimation *ripple_;
    QPointF rippleOrigin_;
};
