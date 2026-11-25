// Copyright [2026] yyamasak
#pragma once
#include <string>

// アプリの出力はすべてここを通す(std::cout / std::cerr を直接使わない)。
//  info  : 通常の案内。標準出力へ
//  warn  : 警告。標準エラーへ
//  error : エラー。標準エラーへ
// エラーを標準エラーに分けるのは、`./scop > log.txt` でも画面に出るようにするため。
class Logger {
 public:
  static void info(const std::string& message);
  static void warn(const std::string& message);
  static void error(const std::string& message);

 private:
  Logger() = delete;  // 静的メソッドだけのクラス
};
