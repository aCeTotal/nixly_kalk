#pragma once

#include <QChar>

enum class VimCommand {
    Unbound,
    Pending,
    Left,
    Down,
    Up,
    Right,
    Press,
    Backspace,
    Clear,
    Copy,
    Paste,
    First,
    Last,
    Leave,
};

/* Normal-mode keys with doubled operators. */
class VimKeys
{
public:
    VimCommand feed(QChar key);

private:
    QChar pending_;
};
