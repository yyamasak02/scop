// Copyright [2026] yyamasak
#pragma once

class MotionMode {
 public:
  enum MotionModeValue { Camera, Object };
  static const char* toName(MotionMode::MotionModeValue value);
};
