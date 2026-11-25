// Copyright [2026] yyamasak
#include "custom/Logger.hpp"

#include <iostream>
#include <string>

void Logger::info(const std::string& message) {
  std::cout << message << std::endl;
}

void Logger::warn(const std::string& message) {
  std::cerr << "[WARN] " << message << std::endl;
}

void Logger::error(const std::string& message) {
  std::cerr << "[ERROR] " << message << std::endl;
}
