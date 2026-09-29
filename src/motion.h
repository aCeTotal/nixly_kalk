#pragma once

#include <QVariantAnimation>
#include <QWidget>

/* Zero-to-one animation repainting target. */
inline QVariantAnimation *unitAnimation(QWidget *target, int duration, QEasingCurve::Type easing)
{
    auto *animation = new QVariantAnimation(target);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setDuration(duration);
    animation->setEasingCurve(easing);
    QObject::connect(animation, &QVariantAnimation::valueChanged, target, [target] { target->update(); });
    return animation;
}
