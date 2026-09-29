#pragma once

#include "text/notation.h"

#include <QString>
#include <QStringView>

/* Pasted text as internal expression. */
QString sanitize(QStringView text, const Notation &notation);
