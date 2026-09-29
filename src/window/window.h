#pragma once

#include "entry/entry.h"
#include "keypad/layout.h"
#include "window/vim.h"

#include <QWidget>

class Keypad;
class ModeBar;
class QStackedWidget;
class Readout;

class Window : public QWidget
{
public:
    explicit Window(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void press(Keystroke stroke);
    void setMode(Mode mode);
    void copy() const;
    void paste();
    void save() const;
    Keypad *keypadFor(Mode mode) const;
    VimCommand commandFor(const QKeyEvent &event, const Keypad &keypad);
    void run(VimCommand command, Keypad *keypad);

    Entry entry_;
    VimKeys vim_;
    Mode mode_ = Mode::Standard;
    ModeBar *modeBar_ = nullptr;
    Readout *readout_ = nullptr;
    QStackedWidget *keypads_ = nullptr;
    Keypad *standard_ = nullptr;
    Keypad *scientific_ = nullptr;
};
