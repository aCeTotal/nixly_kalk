#pragma once

#include "entry/keystroke.h"

#include <optional>

class QKeyEvent;

std::optional<Keystroke> keystrokeFor(const QKeyEvent &event);
