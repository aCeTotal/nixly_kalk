#pragma once

#include <QStringView>

/* Index where trailing number begins. */
qsizetype numberStart(QStringView text);
qsizetype mantissaDigits(QStringView number);
/* Characters one backspace removes. */
qsizetype unitLength(QStringView text);
