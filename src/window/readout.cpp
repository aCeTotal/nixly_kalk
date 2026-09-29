#include "window/readout.h"

#include "motion.h"
#include "palette.h"
#include "text/phrase.h"
#include "text/pretty.h"

#include <QFontMetricsF>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

constexpr int kHeight = 128;
constexpr qreal kPadding = 4;
constexpr qreal kTopY = 12;
constexpr qreal kTopHeight = 28;
constexpr qreal kMainY = 44;
constexpr qreal kMainHeight = 76;
constexpr int kTopPixels = 17;
constexpr int kMainPixels = 46;
constexpr int kFailurePixels = 28;
constexpr int kMinPixels = 22;
constexpr int kPreviewAlpha = 217;
constexpr int kRiseMs = 220;
constexpr int kShakeMs = 320;
constexpr qreal kRiseDistance = 14;
constexpr qreal kShakeCycles = 4;
constexpr qreal kShakeAmplitude = 8;
constexpr QStringView kEquals = u" =";
constexpr Qt::Alignment kRight = Qt::AlignRight | Qt::AlignVCenter;

Phrase phraseOf(Fault fault)
{
    switch (fault) {
    case Fault::DivideByZero:
        return Phrase::DivideByZero;
    case Fault::Domain:
        return Phrase::Domain;
    case Fault::Overflow:
        return Phrase::Overflow;
    default:
        return Phrase::Syntax;
    }
}

qreal progressOf(const QVariantAnimation *animation)
{
    if (animation->state() != QAbstractAnimation::Running)
        return 1;
    return animation->currentValue().toReal();
}

} // namespace

Readout::Readout(QWidget *parent)
    : QWidget(parent)
    , rise_(unitAnimation(this, kRiseMs, QEasingCurve::OutCubic))
    , shake_(unitAnimation(this, kShakeMs, QEasingCurve::Linear))
{
    setFixedHeight(kHeight);
    topFont_.setPixelSize(kTopPixels);
    mainFont_.setWeight(QFont::Light);
    mainFont_.setFeature(QFont::Tag("tnum"), 1);
    topFont_.setFeature(QFont::Tag("tnum"), 1);
}

void Readout::present(const Entry &entry)
{
    const Phase previous = phase_;
    phase_ = entry.phase();
    if (phase_ == Phase::Editing)
        showEditing(entry);
    else
        showEquation(entry);
    fit();
    if (phase_ != previous && phase_ != Phase::Editing) {
        rise_->stop();
        rise_->start();
    }
    update();
}

void Readout::shake()
{
    shake_->stop();
    shake_->start();
}

void Readout::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal rise = progressOf(rise_);
    const qreal span = width() - 2 * kPadding;
    painter.translate(shakeOffset(), 0);
    painter.setOpacity(rise);
    painter.setFont(topFont_);
    painter.setPen(topColor_);
    painter.drawText(QRectF(kPadding, kTopY, span, kTopHeight), kRight, topShown_);

    const QRectF line(kPadding, kMainY + kRiseDistance * (1 - rise), span, kMainHeight);
    painter.setFont(mainFont_);
    painter.setPen(palette::dim);
    painter.drawText(line, kRight, ghost_);
    painter.setPen(mainColor_);
    painter.drawText(line.adjusted(0, 0, -ghostWidth_, 0), kRight, mainShown_);
}

void Readout::resizeEvent(QResizeEvent *event)
{
    fit();
    QWidget::resizeEvent(event);
}

void Readout::showEditing(const Entry &entry)
{
    const Notation &notation = systemNotation();
    top_ = prettify(entry.preview(), notation);
    topColor_ = palette::withAlpha(palette::accent, kPreviewAlpha);
    main_ = entry.text().isEmpty() ? QStringLiteral("0") : prettify(entry.text(), notation);
    ghost_ = QString(entry.openParens(), u')');
    mainColor_ = palette::text;
    mainPixels_ = kMainPixels;
}

void Readout::showEquation(const Entry &entry)
{
    const Notation &notation = systemNotation();
    top_ = prettify(entry.equation(), notation) + kEquals;
    topColor_ = palette::dim;
    ghost_.clear();
    if (entry.phase() == Phase::Result) {
        main_ = prettify(entry.text(), notation);
        mainColor_ = palette::text;
        mainPixels_ = kMainPixels;
        return;
    }
    main_ = phrase(phraseOf(entry.fault())).toString();
    mainColor_ = palette::danger;
    mainPixels_ = kFailurePixels;
}

void Readout::fit()
{
    const qreal available = width() - 2 * kPadding;
    mainFont_.setPixelSize(mainPixels_);
    const QFontMetricsF natural(mainFont_);
    const qreal needed = natural.horizontalAdvance(main_) + natural.horizontalAdvance(ghost_);
    if (needed > available)
        mainFont_.setPixelSize(std::max(kMinPixels, static_cast<int>(mainPixels_ * available / needed)));

    const QFontMetricsF fitted(mainFont_);
    ghostWidth_ = fitted.horizontalAdvance(ghost_);
    mainShown_ = fitted.elidedText(main_, Qt::ElideLeft, available - ghostWidth_);
    topShown_ = QFontMetricsF(topFont_).elidedText(top_, Qt::ElideLeft, available);
}

qreal Readout::shakeOffset() const
{
    if (shake_->state() != QAbstractAnimation::Running)
        return 0;
    const qreal progress = shake_->currentValue().toReal();
    return std::sin(progress * kShakeCycles * 2 * std::numbers::pi) * kShakeAmplitude * (1 - progress);
}
