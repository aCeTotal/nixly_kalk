#pragma once

#include <QStringView>

enum class Command {
    Digit,
    Point,
    Operator,
    Open,
    Close,
    Postfix,
    Function,
    Constant,
    Raise,
    Negate,
    Backspace,
    Clear,
    Equals,
    Angle,
};

/* Token views static literals. */
struct Keystroke {
    Command command;
    QStringView token;
};
