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

private:
    void press(Key *key);
    Key *find(Command command) const;

    std::vector<Key *> keys_;
    std::function<void(Keystroke)> onPress_;
};
