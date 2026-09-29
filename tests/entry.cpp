#include "check.h"

#include "entry/entry.h"

#include <QString>
#include <initializer_list>

namespace {

const Notation kNorwegian{QStringLiteral(","), QStringLiteral(" ")};

bool press(Entry &entry, Command command, QStringView token = {})
{
    return entry.apply({command, token});
}

void digits(Entry &entry, std::initializer_list<QStringView> tokens)
{
    for (QStringView token : tokens)
        press(entry, Command::Digit, token);
}

void editing()
{
    Entry entry;
    digits(entry, {u"0", u"1", u"2"});
    check(entry.text() == u"12", "leading zero replaced");
    check(entry.preview().isEmpty(), "lone number has no preview");
    press(entry, Command::Operator, u"+");
    press(entry, Command::Operator, u"*");
    check(entry.text() == u"12*", "operator replaced");
    press(entry, Command::Operator, u"-");
    check(entry.text() == u"12*-", "unary minus after times");
    press(entry, Command::Operator, u"+");
    check(entry.text() == u"12+", "operator run replaced");
    press(entry, Command::Digit, u"3");
    check(entry.preview() == u"15", "live preview");
    check(!press(entry, Command::Close), "close without open");
    press(entry, Command::Clear);
    check(entry.text().isEmpty() && entry.preview().isEmpty(), "clear");
}

void emptyStart()
{
    Entry entry;
    press(entry, Command::Operator, u"*");
    check(entry.text() == u"0*", "times on empty");
    press(entry, Command::Clear);
    press(entry, Command::Operator, u"-");
    check(entry.text() == u"-", "minus on empty");
    press(entry, Command::Clear);
    press(entry, Command::Point);
    check(entry.text() == u"0.", "point on empty");
    press(entry, Command::Digit, u"5");
    check(!press(entry, Command::Point), "second point");
    press(entry, Command::Clear);
    press(entry, Command::Open);
    check(!press(entry, Command::Operator, u"+") && entry.text() == u"(", "plus after open");
    check(press(entry, Command::Operator, u"-") && entry.text() == u"(-", "minus after open");
}

void results()
{
    Entry entry;
    digits(entry, {u"2"});
    press(entry, Command::Operator, u"*");
    press(entry, Command::Open);
    digits(entry, {u"3"});
    press(entry, Command::Operator, u"+");
    press(entry, Command::Open);
    digits(entry, {u"4"});
    check(entry.openParens() == 2, "open parens");
    check(press(entry, Command::Equals), "equals");
    check(entry.phase() == Phase::Result && entry.text() == u"14", "result");
    check(entry.equation() == u"2*(3+(4))", "equation closed");
    press(entry, Command::Operator, u"+");
    check(entry.phase() == Phase::Editing && entry.text() == u"14+", "operator continues");
    press(entry, Command::Equals);
    check(entry.text() == u"14+", "syntax error keeps state");
    press(entry, Command::Backspace);
    press(entry, Command::Equals);
    press(entry, Command::Digit, u"7");
    check(entry.text() == u"7", "digit starts new");
    press(entry, Command::Equals);
    press(entry, Command::Function, u"√");
    check(entry.text() == u"√(7)", "function wraps result");
}

void faults()
{
    Entry entry;
    digits(entry, {u"5"});
    press(entry, Command::Operator, u"/");
    digits(entry, {u"0"});
    press(entry, Command::Equals);
    check(entry.phase() == Phase::Failed && entry.fault() == Fault::DivideByZero, "divide by zero");
    press(entry, Command::Digit, u"2");
    check(entry.phase() == Phase::Editing && entry.text() == u"2", "recover with digit");
}

void negation()
{
    Entry entry;
    digits(entry, {u"5"});
    press(entry, Command::Negate);
    check(entry.text() == u"-5", "negate first");
    press(entry, Command::Negate);
    check(entry.text() == u"5", "negate back");
    press(entry, Command::Operator, u"-");
    digits(entry, {u"3"});
    press(entry, Command::Negate);
    check(entry.text() == u"5-(-3", "negate inner");
    press(entry, Command::Negate);
    check(entry.text() == u"5-3", "negate inner back");
    press(entry, Command::Operator, u"*");
    press(entry, Command::Operator, u"-");
    digits(entry, {u"2"});
    press(entry, Command::Negate);
    check(entry.text() == u"5-3*2", "negate unary minus");
}

void editingUnits()
{
    Entry entry;
    press(entry, Command::Function, u"asin");
    press(entry, Command::Backspace);
    check(entry.text().isEmpty(), "backspace arcsine");
    press(entry, Command::Function, u"√");
    press(entry, Command::Backspace);
    check(entry.text().isEmpty(), "backspace root");
    digits(entry, {u"2"});
    press(entry, Command::Raise, u"10");
    check(entry.text() == u"2*10^", "raise");
    press(entry, Command::Clear);
    for (int i = 0; i < 16; ++i)
        press(entry, Command::Digit, u"9");
    check(!press(entry, Command::Digit, u"9"), "digit limit");
}

void angles()
{
    Entry entry;
    press(entry, Command::Function, u"sin");
    digits(entry, {u"9", u"0"});
    check(entry.preview() == u"1", "degrees preview");
    entry.setAngle(Angle::Radians);
    check(entry.preview() == u"0.8939966636005579", "radians preview");
}

void pasting()
{
    Entry entry;
    entry.paste(u"1 234,5 × 2", kNorwegian);
    check(entry.text() == u"1234.5*2", "paste sanitized");
    entry.paste(u"x = 7!?", kNorwegian);
    check(entry.text() == u"1234.5*27!", "paste filters alphabet");
}

} // namespace

int main()
{
    editing();
    emptyStart();
    results();
    faults();
    negation();
    editingUnits();
    angles();
    pasting();
    return failures == 0 ? 0 : 1;
}
