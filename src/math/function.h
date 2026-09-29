#pragma once

#include "math/evaluate.h"

#include <QStringView>
#include <span>

struct Function {
    QStringView name;
    long double (*apply)(long double x, Angle angle);
};

/* Named functions, prefix-free names. */
std::span<const Function> functions();
