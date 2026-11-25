// Copyright [2026] yyamasak
#include "custom/Logger.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <source_location>
#include <string>

namespace {

// "2026-11-25 21:50:12.345"
std::string timestamp() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const std::time_t t = system_clock::to_time_t(now);
  const int ms = static_cast<int>(
      duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
  std::tm tm{};
  localtime_r(&t, &tm);  // スレッドセーフ版
  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
  char out[48];
  std::snprintf(out, sizeof(out), "%s.%03d", buf, ms);
  return out;
}

// function_name() は "void Shader::use() const" のような署名全体を返すので、
// 引数リストと戻り値の型を落として "Shader::use" だけにする。
std::string callerName(const std::source_location& where) {
  std::string name = where.function_name();
  const size_t paren = name.find('(');
  if (paren != std::string::npos) name.erase(paren);
  const size_t space = name.rfind(' ');
  if (space != std::string::npos) name.erase(0, space + 1);
  return name;
}

void write(std::ostream& os, const char* level, const std::string& message,
           const std::source_location& where) {
  os << timestamp() << " [" << level << "] [" << callerName(where) << "] "
     << message << std::endl;
}

}  // namespace

void Logger::info(const std::string& message, std::source_location where) {
  write(std::cout, "INFO ", message, where);
}

void Logger::warn(const std::string& message, std::source_location where) {
  write(std::cerr, "WARN ", message, where);
}

void Logger::error(const std::string& message, std::source_location where) {
  write(std::cerr, "ERROR", message, where);
}
