#include "window/window.h"

#include "keypad/keypad.h"
#include "palette.h"
#include "text/phrase.h"
#include "window/keyboard.h"
#include "window/modebar.h"
#include "window/readout.h"

#include <QClipboard>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QSettings>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

constexpr int kWidth = 320;
constexpr int kHeight = 520;
constexpr int kMargin = 14;
constexpr int kSpacing = 10;
constexpr int kFadeMs = 160;
constexpr int kFar = 1000;
constexpr QStringView kVimEntry = u"hjk";
constexpr qreal kGlowX = 0.85;
constexpr qreal kGlowY = 40;
constexpr qreal kGlowRadius = 260;
constexpr auto kSettingsName = "nixlykalk";
constexpr auto kModeKey = "mode";
constexpr auto kAngleKey = "angle";

template<typename Enum>
Enum stored(const QSettings &settings, const char *key, Enum last)
{
    const int value = settings.value(key, 0).toInt();
    if (value < 0 || value > static_cast<int>(last))
        return Enum{};
    return static_cast<Enum>(value);
}

std::optional<Mode> modeFor(const QKeyEvent &event)
{
    if (event.modifiers() != Qt::AltModifier)
        return std::nullopt;
    switch (event.key()) {
    case Qt::Key_1:
        return Mode::Standard;
    case Qt::Key_2:
        return Mode::Scientific;
    default:
        return std::nullopt;
    }
}

std::optional<VimCommand> arrowFor(int key)
{
    switch (key) {
    case Qt::Key_Up:
        return VimCommand::Up;
    case Qt::Key_Down:
        return VimCommand::Down;
    case Qt::Key_Left:
        return VimCommand::Left;
    case Qt::Key_Right:
        return VimCommand::Right;
    default:
        return std::nullopt;
    }
}

bool isEnter(int key)
{
    return key == Qt::Key_Return || key == Qt::Key_Enter;
}

void fadeIn(QWidget *widget)
{
    auto *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto *fade = new QPropertyAnimation(effect, "opacity", widget);
    fade->setDuration(kFadeMs);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    QObject::connect(fade, &QAbstractAnimation::finished, widget, [widget] { widget->setGraphicsEffect(nullptr); });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWidth, kHeight);
    setWindowTitle(phrase(Phrase::Title).toString());

    const QSettings settings(kSettingsName, kSettingsName);
    mode_ = stored(settings, kModeKey, Mode::Scientific);
    entry_.setAngle(stored(settings, kAngleKey, Angle::Radians));

    const auto onPress = [this](Keystroke stroke) { press(stroke); };
    modeBar_ = new ModeBar(mode_, [this](Mode mode) { setMode(mode); }, this);
    readout_ = new Readout([this] { copy(); }, [this] { press({Command::Clear, {}}); }, this);
    keypads_ = new QStackedWidget(this);
    standard_ = new Keypad(Mode::Standard, onPress, keypads_);
    scientific_ = new Keypad(Mode::Scientific, onPress, keypads_);
    keypads_->addWidget(standard_);
    keypads_->addWidget(scientific_);
    keypads_->setCurrentWidget(keypadFor(mode_));
    standard_->setAngle(entry_.angle());
    scientific_->setAngle(entry_.angle());

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    layout->setSpacing(kSpacing);
    layout->addWidget(modeBar_);
    layout->addWidget(readout_);
    layout->addWidget(keypads_, 1);
    readout_->present(entry_);
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy)) {
        copy();
        return;
    }
    if (event->matches(QKeySequence::Paste)) {
        paste();
        return;
    }
    if (const std::optional<Mode> mode = modeFor(*event)) {
        setMode(*mode);
        return;
    }
    Keypad *keypad = keypadFor(mode_);
    const VimCommand command = commandFor(*event, *keypad);
    if (command != VimCommand::Unbound) {
        run(command, keypad);
        return;
    }
    const std::optional<Keystroke> stroke = keystrokeFor(*event);
    if (!stroke) {
        QWidget::keyPressEvent(event);
        return;
    }
    keypad->hideCursor();
    keypad->flash(*stroke);
    press(*stroke);
}

void Window::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette::background);
    QRadialGradient glow(QPointF(width() * kGlowX, kGlowY), kGlowRadius);
    glow.setColorAt(0, palette::glow);
    glow.setColorAt(1, Qt::transparent);
    painter.fillRect(rect(), glow);
    painter.setPen(palette::hairline);
    painter.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5));
}

void Window::press(Keystroke stroke)
{
    if (!entry_.apply(stroke))
        readout_->shake();
    readout_->present(entry_);
    if (stroke.command != Command::Angle)
        return;
    standard_->setAngle(entry_.angle());
    scientific_->setAngle(entry_.angle());
    save();
}

VimCommand Window::commandFor(const QKeyEvent &event, const Keypad &keypad)
{
    if (const std::optional<VimCommand> arrow = arrowFor(event.key()))
        return *arrow;
    const bool navigating = keypad.navigating();
    if (navigating && event.key() == Qt::Key_Escape)
        return VimCommand::Leave;
    if (navigating && isEnter(event.key()))
        return VimCommand::Press;
    const QString text = event.text();
    if (text.isEmpty() || (!navigating && !kVimEntry.contains(text.front())))
        return VimCommand::Unbound;
    return vim_.feed(text.front());
}

void Window::run(VimCommand command, Keypad *keypad)
{
    switch (command) {
    case VimCommand::Left:
        return keypad->step(0, -1);
    case VimCommand::Down:
        return keypad->step(1, 0);
    case VimCommand::Up:
        return keypad->step(-1, 0);
    case VimCommand::Right:
        return keypad->step(0, 1);
    case VimCommand::First:
        return keypad->step(-kFar, -kFar);
    case VimCommand::Last:
        return keypad->step(kFar, kFar);
    case VimCommand::Press:
        keypad->activate();
        return;
    case VimCommand::Backspace:
        return press({Command::Backspace, {}});
    case VimCommand::Clear:
        return press({Command::Clear, {}});
    case VimCommand::Copy:
        return copy();
    case VimCommand::Paste:
        return paste();
    case VimCommand::Leave:
        return keypad->hideCursor();
    case VimCommand::Unbound:
    case VimCommand::Pending:
        return;
    }
}

void Window::setMode(Mode mode)
{
    if (mode == mode_)
        return;
    keypadFor(mode_)->hideCursor();
    mode_ = mode;
    Keypad *incoming = keypadFor(mode);
    keypads_->setCurrentWidget(incoming);
    modeBar_->setMode(mode);
    fadeIn(incoming);
    save();
}

void Window::copy() const
{
    const bool previewing = entry_.phase() == Phase::Editing && !entry_.preview().isEmpty();
    QString number = previewing ? entry_.preview() : entry_.text();
    number.replace(u'.', systemNotation().decimal);
    QGuiApplication::clipboard()->setText(number);
}

void Window::paste()
{
    entry_.paste(QGuiApplication::clipboard()->text(), systemNotation());
    readout_->present(entry_);
}

void Window::save() const
{
    QSettings settings(kSettingsName, kSettingsName);
    settings.setValue(kModeKey, static_cast<int>(mode_));
    settings.setValue(kAngleKey, static_cast<int>(entry_.angle()));
}

Keypad *Window::keypadFor(Mode mode) const
{
    return mode == Mode::Standard ? standard_ : scientific_;
}
