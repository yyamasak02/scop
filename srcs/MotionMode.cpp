// Copyright [2026] yyamasak
#include "custom/MotionMode.hpp"

const char* MotionMode::toName(MotionMode::MotionModeValue value) {
  switch (value) {
    case MotionMode::Camera:
      return "Camera";
    case MotionMode::Object:
      return "Object";
  }
  return "Unknown";
}
