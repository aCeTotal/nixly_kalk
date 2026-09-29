#pragma once

#include "entry/keystroke.h"

#include <QStringView>
#include <span>

enum class Mode { Standard, Scientific };

enum class Kind { Digit, Function, Operator, Clear, Equals, Shift };

struct KeySpec {
    QStringView label;
    Keystroke stroke;
    QStringView altLabel;
    Keystroke altStroke;
    Kind kind;
};

/* Row-major keys for one mode. */
std::span<const KeySpec> keysOf(Mode mode);
int columnsOf(Mode mode);
