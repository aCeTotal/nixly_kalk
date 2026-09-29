#pragma once

#include "keypad/layout.h"
#include "math/evaluate.h"

#include <QWidget>
#include <functional>
#include <vector>

class Key;

class Keypad : public QWidget
{
public:
    Keypad(Mode mode, std::function<void(Keystroke)> onPress, QWidget *parent = nullptr);

    void flash(Keystroke stroke);
    void setAngle(Angle angle);
    void step(int rows, int columns);
    bool activate();
    void hideCursor();
    bool navigating() const { return cursor_ >= 0; }

private:
    void press(Key *key);
    Key *find(Command command) const;
    void select(int index);

    std::vector<Key *> keys_;
    std::function<void(Keystroke)> onPress_;
    int columns_;
    int home_ = 0;
    int cursor_ = -1;
};
