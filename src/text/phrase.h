#pragma once

#include <QStringView>

enum class Phrase { Title, Standard, Scientific, Syntax, DivideByZero, Domain, Overflow };

/* In the system UI language. */
QStringView phrase(Phrase phrase);
