#pragma once

#include <QString>

struct Notation {
    QString decimal;
    QString group;
};

/* Separators from LC_NUMERIC. */
const Notation &systemNotation();
