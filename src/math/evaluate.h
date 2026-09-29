#pragma once

#include <QStringView>

enum class Angle { Degrees, Radians };

enum class Fault { None, Syntax, DivideByZero, Domain, Overflow };

struct Evaluation {
    long double value;
    Fault fault;
};

/* Unclosed parentheses close at the end. */
Evaluation evaluate(QStringView expression, Angle angle);
