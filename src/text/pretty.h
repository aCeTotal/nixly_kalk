#pragma once

#include "text/notation.h"

#include <QString>
#include <QStringView>

/* Internal expression as displayed. */
QString prettify(QStringView expression, const Notation &notation);
