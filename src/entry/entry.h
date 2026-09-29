#pragma once

#include "entry/keystroke.h"
#include "math/evaluate.h"
#include "text/notation.h"

#include <QString>

enum class Phase { Editing, Result, Failed };

class Entry
{
public:
    /* False means rejected keystroke. */
    bool apply(Keystroke stroke);
    void paste(QStringView text, const Notation &notation);
    void setAngle(Angle angle);

    const QString &text() const { return text_; }
    const QString &equation() const { return equation_; }
    const QString &preview() const { return preview_; }
    Phase phase() const { return phase_; }
    Fault fault() const { return fault_; }
    Angle angle() const { return angle_; }
    int openParens() const;

private:
    bool route(Keystroke stroke);
    bool resume(Keystroke stroke);
    bool recover(Keystroke stroke);
    bool edit(Keystroke stroke);
    bool dispatch(Keystroke stroke);
    bool digit(QStringView token);
    bool point();
    bool binary(QChar op);
    bool begin(QChar op);
    bool appendMinus(QChar op);
    bool replaceOperators(QChar op);
    bool close();
    bool postfix(QStringView token);
    bool raise(QStringView base);
    bool negate();
    bool negateEmpty();
    bool equals();
    bool endsWithOperand() const;
    void reset();
    void refresh();

    QString text_;
    QString equation_;
    QString preview_;
    Phase phase_ = Phase::Editing;
    Fault fault_ = Fault::None;
    Angle angle_ = Angle::Degrees;
};
