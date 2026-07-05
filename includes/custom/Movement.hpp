// Copyright [2026] yyamasak
#pragma once

class Movement {
 public:
  enum MovementValue { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };
  static const char* toName(Movement::MovementValue value);
};
