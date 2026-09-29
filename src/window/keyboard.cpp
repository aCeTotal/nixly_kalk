#include "window/keyboard.h"

#include <QKeyEvent>
#include <algorithm>
#include <array>

namespace {

struct Binding {
    char16_t key;
    Keystroke stroke;
};

constexpr std::array<QStringView, 10> kDigits{u"0", u"1", u"2", u"3", u"4", u"5", u"6", u"7", u"8", u"9"};

constexpr std::array kBindings{
    Binding{u'.', {Command::Point, u"."}},
    Binding{u',', {Command::Point, u"."}},
    Binding{u'+', {Command::Operator, u"+"}},
    Binding{u'-', {Command::Operator, u"-"}},
    Binding{u'−', {Command::Operator, u"-"}},
    Binding{u'*', {Command::Operator, u"*"}},
    Binding{u'x', {Command::Operator, u"*"}},
    Binding{u'×', {Command::Operator, u"*"}},
    Binding{u'/', {Command::Operator, u"/"}},
    Binding{u'÷', {Command::Operator, u"/"}},
    Binding{u'^', {Command::Operator, u"^"}},
    Binding{u'(', {Command::Open, u"("}},
    Binding{u')', {Command::Close, u")"}},
    Binding{u'!', {Command::Postfix, u"!"}},
    Binding{u'%', {Command::Postfix, u"%"}},
    Binding{u'²', {Command::Postfix, u"²"}},
    Binding{u'=', {Command::Equals, {}}},
    Binding{u's', {Command::Function, u"sin"}},
    Binding{u'c', {Command::Function, u"cos"}},
    Binding{u't', {Command::Function, u"tan"}},
    Binding{u'l', {Command::Function, u"ln"}},
    Binding{u'g', {Command::Function, u"log"}},
    Binding{u'r', {Command::Function, u"√"}},
    Binding{u'p', {Command::Constant, u"π"}},
    Binding{u'e', {Command::Constant, u"e"}},
};

constexpr Qt::KeyboardModifiers kChordModifiers = Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;

std::optional<Keystroke> strokeForKey(int key)
{
    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return Keystroke{Command::Equals, {}};
    case Qt::Key_Backspace:
        return Keystroke{Command::Backspace, {}};
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        return Keystroke{Command::Clear, {}};
    default:
        return std::nullopt;
    }
}

std::optional<Keystroke> strokeForChar(QChar c)
{
    if (c.isDigit())
        return Keystroke{Command::Digit, kDigits[static_cast<std::size_t>(c.digitValue())]};
    const auto match = std::ranges::find(kBindings, c.unicode(), &Binding::key);
    if (match == kBindings.end())
        return std::nullopt;
    return match->stroke;
}

} // namespace

std::optional<Keystroke> keystrokeFor(const QKeyEvent &event)
{
    if (event.modifiers() & kChordModifiers)
        return std::nullopt;
    if (const std::optional<Keystroke> stroke = strokeForKey(event.key()))
        return stroke;
    const QString text = event.text();
    if (text.isEmpty())
        return std::nullopt;
    return strokeForChar(text.front());
}
