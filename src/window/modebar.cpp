#include "window/modebar.h"

#include "palette.h"
#include "text/phrase.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>
#include <array>

namespace {

constexpr int kHeight = 34;
constexpr int kFontSize = 13;
constexpr int kSlideMs = 200;
constexpr qreal kTrackRadius = 10;
constexpr qreal kPillRadius = 8;
constexpr qreal kPillInset = 3;
constexpr int kPillFillAlpha = 60;
constexpr int kPillBorderAlpha = 140;
constexpr std::array kModes{Mode::Standard, Mode::Scientific};


qreal indexOf(Mode mode)
{
    return mode == Mode::Standard ? 0 : 1;
}

} // namespace

ModeBar::ModeBar(Mode mode, std::function<void(Mode)> onChange, QWidget *parent)
    : QWidget(parent)
    , mode_(mode)
    , onChange_(std::move(onChange))
    , slide_(new QVariantAnimation(this))
    , position_(indexOf(mode))
    , labels_{phrase(Phrase::Standard).toString(), phrase(Phrase::Scientific).toString()}
{
    setFixedHeight(kHeight);
    setCursor(Qt::PointingHandCursor);
    font_.setPixelSize(kFontSize);
    font_.setWeight(QFont::Medium);
    slide_->setDuration(kSlideMs);
    slide_->setEasingCurve(QEasingCurve::OutCubic);
    connect(slide_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        position_ = value.toReal();
        update();
    });
}

void ModeBar::setMode(Mode mode)
{
    if (mode == mode_)
        return;
    mode_ = mode;
    slide_->stop();
    slide_->setStartValue(position_);
    slide_->setEndValue(indexOf(mode));
    slide_->start();
}

void ModeBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF track = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(palette::hairline);
    painter.setBrush(palette::track);
    painter.drawRoundedRect(track, kTrackRadius, kTrackRadius);

    const qreal segmentWidth = track.width() / kModes.size();
    const QRectF pill(track.left() + position_ * segmentWidth, track.top(), segmentWidth, track.height());
    painter.setPen(palette::withAlpha(palette::accent, kPillBorderAlpha));
    painter.setBrush(palette::withAlpha(palette::accent, kPillFillAlpha));
    painter.drawRoundedRect(pill.adjusted(kPillInset, kPillInset, -kPillInset, -kPillInset), kPillRadius, kPillRadius);

    painter.setFont(font_);
    for (std::size_t i = 0; i < kModes.size(); ++i) {
        const QRectF segment(track.left() + i * segmentWidth, track.top(), segmentWidth, track.height());
        painter.setPen(kModes[i] == mode_ ? palette::text : palette::dim);
        painter.drawText(segment, Qt::AlignCenter, labels_[i]);
    }
}

void ModeBar::mousePressEvent(QMouseEvent *event)
{
    const Mode clicked = event->position().x() < width() / 2.0 ? Mode::Standard : Mode::Scientific;
    if (clicked != mode_)
        onChange_(clicked);
}
