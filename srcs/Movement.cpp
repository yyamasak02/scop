// Copyright [2026] yyamasak
#include "custom/Movement.hpp"

const char* Movement::toName(Movement::MovementValue value) {
  switch (value) {
    case Movement::FORWARD:
      return "Forward";
    case Movement::BACKWARD:
      return "Backward";
    case Movement::RIGHT:
      return "Right";
    case Movement::LEFT:
      return "Left";
    case Movement::UP:
      return "Up";
    case Movement::DOWN:
      return "Down";
  }
  return "Unknown";
}
