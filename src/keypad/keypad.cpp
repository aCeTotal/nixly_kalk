#include "keypad/keypad.h"

#include "keypad/key.h"
#include "text/notation.h"

#include <QGridLayout>
#include <algorithm>

namespace {

constexpr int kStandardSpacing = 8;
constexpr int kScientificSpacing = 6;
constexpr QStringView kHomeDigit = u"5";

} // namespace

Keypad::Keypad(Mode mode, std::function<void(Keystroke)> onPress, QWidget *parent)
    : QWidget(parent)
    , onPress_(std::move(onPress))
    , columns_(columnsOf(mode))
{
    auto *grid = new QGridLayout(this);
    grid->setContentsMargins({});
    grid->setSpacing(mode == Mode::Standard ? kStandardSpacing : kScientificSpacing);

    const std::span<const KeySpec> specs = keysOf(mode);
    keys_.reserve(specs.size());
    for (const KeySpec &spec : specs) {
        const int index = static_cast<int>(keys_.size());
        Key *key = new Key(spec, this);
        grid->addWidget(key, index / columns_, index % columns_);
        connect(key, &QAbstractButton::clicked, this, [this, key] { press(key); });
        keys_.push_back(key);
        if (spec.stroke.token == kHomeDigit)
            home_ = index;
    }
    find(Command::Point)->setLabel(systemNotation().decimal);
}

void Keypad::step(int rows, int columns)
{
    if (cursor_ < 0) {
        select(home_);
        return;
    }
    const int lastRow = static_cast<int>(keys_.size()) / columns_ - 1;
    const int row = std::clamp(cursor_ / columns_ + rows, 0, lastRow);
    const int column = std::clamp(cursor_ % columns_ + columns, 0, columns_ - 1);
    select(row * columns_ + column);
}

bool Keypad::activate()
{
    if (cursor_ < 0)
        return false;
    Key *key = keys_[static_cast<std::size_t>(cursor_)];
    key->flash();
    key->click();
    return true;
}

void Keypad::hideCursor()
{
    select(-1);
}

void Keypad::select(int index)
{
    if (cursor_ >= 0)
        keys_[static_cast<std::size_t>(cursor_)]->setSelected(false);
    cursor_ = index;
    if (cursor_ >= 0)
        keys_[static_cast<std::size_t>(cursor_)]->setSelected(true);
}

void Keypad::flash(Keystroke stroke)
{
    const auto match = std::ranges::find_if(keys_, [stroke](const Key *key) {
        const Keystroke own = key->stroke();
        return key->spec().kind != Kind::Shift && own.command == stroke.command && own.token == stroke.token;
    });
    if (match != keys_.end())
        (*match)->flash();
}

void Keypad::setAngle(Angle angle)
{
    Key *key = find(Command::Angle);
    if (key)
        key->setLabel(angle == Angle::Degrees ? QStringLiteral("DEG") : QStringLiteral("RAD"));
}

void Keypad::press(Key *key)
{
    if (key->spec().kind != Kind::Shift) {
        onPress_(key->stroke());
        return;
    }
    for (Key *each : keys_)
        each->toggleAlternate();
}

Key *Keypad::find(Command command) const
{
    const auto match = std::ranges::find_if(keys_, [command](const Key *key) {
        return key->spec().kind != Kind::Shift && key->stroke().command == command;
    });
    return match == keys_.end() ? nullptr : *match;
}
