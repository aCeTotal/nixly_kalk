#include "keypad/keypad.h"

#include "keypad/key.h"
#include "text/notation.h"

#include <QGridLayout>
#include <algorithm>

namespace {

constexpr int kStandardSpacing = 8;
constexpr int kScientificSpacing = 6;

} // namespace

Keypad::Keypad(Mode mode, std::function<void(Keystroke)> onPress, QWidget *parent)
    : QWidget(parent)
    , onPress_(std::move(onPress))
{
    auto *grid = new QGridLayout(this);
    grid->setContentsMargins({});
    grid->setSpacing(mode == Mode::Standard ? kStandardSpacing : kScientificSpacing);

    const std::span<const KeySpec> specs = keysOf(mode);
    const int columns = columnsOf(mode);
    keys_.reserve(specs.size());
    for (const KeySpec &spec : specs) {
        const int index = static_cast<int>(keys_.size());
        Key *key = new Key(spec, this);
        grid->addWidget(key, index / columns, index % columns);
        connect(key, &QAbstractButton::clicked, this, [this, key] { press(key); });
        keys_.push_back(key);
    }
    find(Command::Point)->setLabel(systemNotation().decimal);
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
