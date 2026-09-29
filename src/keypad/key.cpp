#include "keypad/key.h"

#include "motion.h"
#include "palette.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

namespace {

constexpr int kHoverMs = 120;
constexpr int kRippleMs = 380;
constexpr qreal kInset = 2;
constexpr qreal kMaxRadius = 12;
constexpr qreal kRadiusDivisor = 4;
constexpr qreal kPressedScale = 0.96;
constexpr qreal kLargeText = 0.40;
constexpr qreal kSmallText = 0.32;
constexpr int kHoverAlpha = 14;
constexpr int kHighlightAlpha = 14;
constexpr int kRippleAlpha = 60;
constexpr int kGlowAlpha = 46;
constexpr int kFunctionTextAlpha = 217;

QColor fillOf(Kind kind)
{
    switch (kind) {
    case Kind::Digit:
        return palette::digitKey;
    case Kind::Operator:
        return palette::operatorKey;
    case Kind::Clear:
        return palette::clearKey;
    default:
        return palette::functionKey;
    }
}

QColor inkOf(Kind kind)
{
    switch (kind) {
    case Kind::Digit:
        return palette::text;
    case Kind::Operator:
    case Kind::Shift:
        return palette::accent;
    case Kind::Clear:
        return palette::danger;
    case Kind::Equals:
        return palette::ink;
    default:
        return palette::withAlpha(palette::text, kFunctionTextAlpha);
    }
}

QBrush brushOf(Kind kind, const QRectF &face)
{
    if (kind != Kind::Equals)
        return fillOf(kind);
    QLinearGradient gradient(face.topLeft(), face.bottomRight());
    gradient.setColorAt(0, palette::accent);
    gradient.setColorAt(1, palette::accentEnd);
    return gradient;
}

bool isLarge(Kind kind)
{
    return kind == Kind::Digit || kind == Kind::Operator || kind == Kind::Equals;
}

void run(QVariantAnimation *animation, QAbstractAnimation::Direction direction)
{
    animation->setDirection(direction);
    if (animation->state() != QAbstractAnimation::Running)
        animation->start();
}

} // namespace

Key::Key(const KeySpec &spec, QWidget *parent)
    : QAbstractButton(parent)
    , spec_(spec)
    , label_(spec.label.toString())
    , altLabel_(spec.altLabel.toString())
    , hover_(unitAnimation(this, kHoverMs, QEasingCurve::OutCubic))
    , ripple_(unitAnimation(this, kRippleMs, QEasingCurve::OutCubic))
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCheckable(spec.kind == Kind::Shift);
    font_.setWeight(spec.kind == Kind::Digit ? QFont::Normal : QFont::Medium);
}

Keystroke Key::stroke() const
{
    return alternate_ ? spec_.altStroke : spec_.stroke;
}

void Key::toggleAlternate()
{
    if (spec_.altLabel.isEmpty())
        return;
    alternate_ = !alternate_;
    update();
}

void Key::setLabel(const QString &label)
{
    label_ = label;
    update();
}

void Key::flash()
{
    startRipple(rect().center());
}

void Key::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF bounds = rect();
    if (isDown()) {
        painter.translate(bounds.center());
        painter.scale(kPressedScale, kPressedScale);
        painter.translate(-bounds.center());
    }
    const QRectF face = bounds.adjusted(kInset, kInset, -kInset, -kInset);
    const qreal radius = std::min(kMaxRadius, face.height() / kRadiusDivisor);
    paintFace(painter, face, radius);
    paintRipple(painter, face, radius);
    painter.setFont(font_);
    painter.setPen(inkOf(spec_.kind));
    painter.drawText(face, Qt::AlignCenter, alternate_ ? altLabel_ : label_);
}

void Key::enterEvent(QEnterEvent *event)
{
    run(hover_, QAbstractAnimation::Forward);
    QAbstractButton::enterEvent(event);
}

void Key::leaveEvent(QEvent *event)
{
    run(hover_, QAbstractAnimation::Backward);
    QAbstractButton::leaveEvent(event);
}

void Key::mousePressEvent(QMouseEvent *event)
{
    startRipple(event->position());
    QAbstractButton::mousePressEvent(event);
}

void Key::resizeEvent(QResizeEvent *event)
{
    const qreal share = isLarge(spec_.kind) ? kLargeText : kSmallText;
    font_.setPixelSize(std::max(1, qRound(height() * share)));
    QAbstractButton::resizeEvent(event);
}

void Key::paintFace(QPainter &painter, const QRectF &face, qreal radius) const
{
    painter.setPen(Qt::NoPen);
    if (spec_.kind == Kind::Equals) {
        painter.setBrush(palette::withAlpha(palette::accent, kGlowAlpha));
        painter.drawRoundedRect(face.adjusted(-kInset, -kInset, kInset, kInset), radius + kInset, radius + kInset);
    }
    painter.setBrush(brushOf(isChecked() ? Kind::Operator : spec_.kind, face));
    painter.drawRoundedRect(face, radius, radius);
    painter.setBrush(palette::withAlpha(Qt::white, qRound(kHoverAlpha * hover_->currentValue().toReal())));
    painter.drawRoundedRect(face, radius, radius);

    const qreal top = face.top() + 0.5;
    painter.setPen(palette::withAlpha(Qt::white, kHighlightAlpha));
    painter.drawLine(QPointF(face.left() + radius, top), QPointF(face.right() - radius, top));
    if (!isChecked())
        return;
    painter.setPen(palette::accent);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(face.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
}

void Key::paintRipple(QPainter &painter, const QRectF &face, qreal radius) const
{
    if (ripple_->state() != QAbstractAnimation::Running)
        return;
    const qreal progress = ripple_->currentValue().toReal();
    const qreal reach = std::hypot(face.width(), face.height()) * progress;
    QPainterPath clip;
    clip.addRoundedRect(face, radius, radius);
    painter.setClipPath(clip);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette::withAlpha(Qt::white, qRound(kRippleAlpha * (1 - progress))));
    painter.drawEllipse(rippleOrigin_, reach, reach);
    painter.setClipping(false);
}

void Key::startRipple(QPointF origin)
{
    rippleOrigin_ = origin;
    ripple_->stop();
    ripple_->start();
}
